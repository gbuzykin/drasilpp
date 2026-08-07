#include "pipeline_layout.h"

#include "d3d12_logger.h"
#include "descriptor_set.h"
#include "device.h"
#include "pipeline.h"
#include "render_target.h"
#include "shader_module.h"
#include "tables.h"

#include "rel/tables.h"

using namespace drasilpp;
using namespace drasilpp::rel;
using namespace drasilpp::rel::d3d12;

// --------------------------------------------------------
// PipelineLayout class implementation

PipelineLayout::PipelineLayout(Device& device) : device_(util::not_null(&device)) {}

PipelineLayout::~PipelineLayout() {}

bool PipelineLayout::create(const uxs::db::value& config) {
    struct DescriptorRange {
        BindingType binding_type;
        std::uint32_t slot;
        std::uint32_t count;
    };

    struct RootParameter {
        ShaderStage visibility;
        Binding binding;
        union {
            std::uint32_t range_count;
            std::uint32_t slot;
        };
    };

    struct SlotBindingRange {
        std::uint32_t slot;
        std::uint32_t count;
        Binding binding;
    };

    const std::array d3d12_desc_sizes{
        device_->getCbvSrvUavDescriptorSize(),
        device_->getSamplerDescriptorSize(),
    };

    std::uint32_t def_space_index = 0;
    uxs::inline_dynarray<RootParameter, 32> params;
    uxs::inline_dynarray<DescriptorRange, 32> ranges;
    PerBindingType<uxs::inline_dynarray<SlotBindingRange, 32>> binding_ranges;

    uxs::inline_dynarray<D3D12_DESCRIPTOR_RANGE, 32> d3d12_ranges;
    uxs::inline_dynarray<D3D12_ROOT_PARAMETER> d3d12_root_params;

    std::uint32_t first_param_index = 0;
    const auto& descriptor_set_layouts = config.value("descriptor_set_layouts");

    for (const auto& layout : descriptor_set_layouts.as_array()) {
        const auto& list = layout.value("descriptor_list");
        const std::uint32_t space_index = layout.value_or<std::uint32_t>("space_index", def_space_index);
        const std::uint32_t max_sets = layout.value_or<std::uint32_t>("max_sets", 8);
        def_space_index = space_index + 1;

        params.clear();
        ranges.clear();

        std::uint32_t dynamic_resource_index = 0;
        PerBindingType<std::uint32_t> next_slots{};

        for (const auto& desc : list.as_array()) {
            const auto type = parseDescriptorType(desc.value("type").as_string_view());
            const auto binding_type = TBL_DESC_BINDING_TYPE[unsigned(type)];
            const std::uint32_t slot = desc.value_or<std::uint32_t>("slot", next_slots[unsigned(binding_type)]);
            const std::uint32_t desc_count = desc.value_or<std::uint32_t>("count", 1);
            const auto visibility = parseShaderStage(desc.value_or<const char*>("shader_visibility", "ALL"));

            if (desc_count == 0) { continue; }

            if (type >= DescriptorType::CONSTANT_BUFFER_DYNAMIC) {
                for (std::uint32_t n = 0; n < desc_count; ++n, ++dynamic_resource_index) {
                    const auto binding = Binding(binding_type, dynamic_resource_index);
                    params.emplace_back(RootParameter{.visibility = visibility, .binding = binding, .slot = slot + n});
                }
                next_slots[unsigned(binding_type)] = slot + desc_count;
                continue;
            }

            const auto append_range = [&params, &ranges, &next_slots, &heaps = heaps_, desc_count, visibility,
                                       max_sets](auto binding_type, auto slot) {
                const auto heap_type = binding_type == BindingType::SAMPLER ? HeapType::SAMPLER : HeapType::CBV_SRV_UAV;

                auto param_it = std::ranges::find_if(params, [visibility, heap_type](const auto& param) {
                    return !param.binding.isDynamic() && param.visibility == visibility &&
                           param.binding.getHeapType() == heap_type;
                });

                ranges.emplace_back(DescriptorRange{.binding_type = binding_type, .slot = slot, .count = desc_count});

                if (param_it != params.end()) {
                    if (param_it != params.end() - 1) {
                        const auto t = ranges.back();
                        const std::uint32_t range_index = param_it->binding.getOffset();
                        std::copy_backward(ranges.begin() + range_index + param_it->range_count, ranges.end() - 1,
                                           ranges.end());
                        ranges[range_index + param_it->range_count] = t;
                        std::for_each(param_it + 1, params.end(), [](auto& param) {
                            if (!param.binding.isDynamic()) { param.binding.addOffset(1); }
                        });
                    }
                    ++param_it->range_count;
                } else {
                    const auto binding = Binding(heap_type, std::uint32_t(ranges.size() - 1));
                    params.emplace_back(RootParameter{.visibility = visibility, .binding = binding, .range_count = 1});
                }

                next_slots[unsigned(binding_type)] = slot + desc_count;
                heaps[unsigned(heap_type)].total_size += max_sets * desc_count;
            };

            append_range(binding_type, slot);
            if (type == DescriptorType::COMBINED_TEXTURE_SAMPLER) {
                const std::uint32_t sampler_slot = desc.value_or<std::uint32_t>(
                    "sampler_slot", next_slots[unsigned(BindingType::SAMPLER)]);
                append_range(BindingType::SAMPLER, sampler_slot);
            }
        }

        auto& set_layout = set_layouts_.emplace_back(DescriptorSetLayout{
            .param_count = std::uint32_t(params.size()),
            .first_param_index = first_param_index,
            .dynamic_resource_count = dynamic_resource_index,
        });

        first_param_index += std::uint32_t(params.size());

        for (auto& range : binding_ranges) { range.clear(); }

        for (const auto& param : params) {
            if (param.binding.isDynamic()) {
                param_bindings_.emplace_back(param.binding);
                binding_ranges[unsigned(param.binding.getBindingType())].emplace_back(
                    SlotBindingRange{.slot = param.slot, .count = 1, .binding = param.binding});
            } else {
                const auto heap_type = param.binding.getHeapType();
                const std::uint32_t d3d12_desc_size = d3d12_desc_sizes[unsigned(heap_type)];
                std::uint32_t& total_desc_size = set_layout.total_desc_sizes[unsigned(heap_type)];
                param_bindings_.emplace_back(Binding(heap_type, total_desc_size));
                for (const auto& range : std::span{ranges}.subspan(param.binding.getOffset(), param.range_count)) {
                    const auto binding = Binding(param.binding.getBindingType(), total_desc_size);
                    binding_ranges[unsigned(range.binding_type)].emplace_back(
                        SlotBindingRange{.slot = range.slot, .count = range.count, .binding = binding});
                    total_desc_size += range.count * d3d12_desc_size;
                }
            }
        }

        for (std::uint32_t n = 0; n < std::uint32_t(BindingType::TOTAL_COUNT); ++n) {
            const auto heap_type = BindingType(n) == BindingType::SAMPLER ? HeapType::SAMPLER : HeapType::CBV_SRV_UAV;
            const std::uint32_t d3d12_desc_size = d3d12_desc_sizes[unsigned(heap_type)];
            set_layout.slot_binding_offsets[n] = std::uint32_t(slot_bindings_.size());
            for (const auto& range : binding_ranges[n]) {
                auto binding = range.binding;
                const std::size_t slot_pos = set_layout.slot_binding_offsets[n] + range.slot;
                if (slot_pos + range.count > slot_bindings_.size()) { slot_bindings_.resize(slot_pos + range.count); }
                for (std::uint32_t n = 0; n < range.count; ++n) {
                    slot_bindings_[slot_pos + n] = binding;
                    binding.addOffset(d3d12_desc_size);
                }
            }
        }

        for (const auto& range : ranges) {
            d3d12_ranges.emplace_back(D3D12_DESCRIPTOR_RANGE{
                .RangeType = TBL_D3D12_DESC_RANGE_TYPE[unsigned(range.binding_type)],
                .NumDescriptors = UINT(range.count),
                .BaseShaderRegister = UINT(range.slot),
                .RegisterSpace = UINT(space_index),
                .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
            });
        }

        for (const auto& param : params) {
            if (param.binding.isDynamic()) {
                d3d12_root_params.emplace_back(D3D12_ROOT_PARAMETER{
                    .ParameterType = TBL_D3D12_DESC_TYPE[unsigned(param.binding.getBindingType())],
                    .Descriptor = {.ShaderRegister = UINT(param.slot), .RegisterSpace = UINT(space_index)},
                    .ShaderVisibility = TBL_D3D12_VISIBILITY[unsigned(param.visibility)],
                });
            } else {
                d3d12_root_params.emplace_back(D3D12_ROOT_PARAMETER{
                    .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                    .DescriptorTable = {.NumDescriptorRanges = UINT(param.range_count)},
                    .ShaderVisibility = TBL_D3D12_VISIBILITY[unsigned(param.visibility)],
                });
            }
        }
    }

    const D3D12_DESCRIPTOR_RANGE* d3d12_range = d3d12_ranges.data();
    for (auto& d3d12_param : d3d12_root_params) {
        if (d3d12_param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
            d3d12_param.DescriptorTable.pDescriptorRanges = d3d12_range;
            d3d12_range += d3d12_param.DescriptorTable.NumDescriptorRanges;
        }
    }

    util::ref_ptr<ID3DBlob> serialized_root_sig;
    util::ref_ptr<ID3DBlob> errors;
    HRESULT result = ::D3D12SerializeRootSignature(
        constAddressOf(D3D12_ROOT_SIGNATURE_DESC{
            .NumParameters = UINT(d3d12_root_params.size()),
            .pParameters = d3d12_root_params.data(),
            .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
        }),
        D3D_ROOT_SIGNATURE_VERSION_1, serialized_root_sig.reset_and_get_address(), errors.reset_and_get_address());
    const auto error_text = errors ? std::string_view(static_cast<const char*>(errors->GetBufferPointer()),
                                                      errors->GetBufferSize()) :
                                     std::string_view{};
    if (result != S_OK) {
        logError(LOG_D3D12 "{}", error_text);
        return false;
    }

    if (errors) { logWarning(LOG_D3D12 "{}", error_text); }

    result = device_->getD3D12Device()->CreateRootSignature(0, serialized_root_sig->GetBufferPointer(),
                                                            serialized_root_sig->GetBufferSize(),
                                                            IID_PPV_ARGS(root_signature_.reset_and_get_address()));
    if (result != S_OK) {
        logError(LOG_D3D12 "couldn't create root signature: {}", D3D12Result(result));
        return false;
    }

    for (std::uint32_t n = 0; n < std::uint32_t(HeapType::TOTAL_COUNT); ++n) {
        const D3D12_DESCRIPTOR_HEAP_DESC heap_desc{
            .Type = TBL_D3D12_HEAP_TYPES[n],
            .NumDescriptors = UINT(heaps_[n].total_size),
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
            .NodeMask = 0,
        };

        heaps_[n].total_size *= d3d12_desc_sizes[n];

        HRESULT result = device_->getD3D12Device()->CreateDescriptorHeap(
            &heap_desc, IID_PPV_ARGS(heaps_[n].heap.reset_and_get_address()));
        if (result != S_OK) {
            logError(LOG_D3D12 "couldn't create descriptor heap: {}", D3D12Result(result));
            return false;
        }
    }

    return true;
}

bool PipelineLayout::obtainDescriptorSet(std::uint32_t set_layout_index, DescriptorSetHandles& handles) {
    const auto& set_layout = set_layouts_[set_layout_index];

    for (std::uint32_t n = 0; n < std::uint32_t(HeapType::TOTAL_COUNT); ++n) {
        auto& heap = heaps_[n];
        if (heap.total_size - heap.used_size < set_layout.total_desc_sizes[n]) {
            logError(LOG_D3D12 "out of descriptor heap");
            return false;
        }

        handles.base_cpu_handles[n] = heap.heap->GetCPUDescriptorHandleForHeapStart() + heap.used_size;
        handles.base_gpu_handles[n] = heap.heap->GetGPUDescriptorHandleForHeapStart() + heap.used_size;

        heap.used_size += set_layout.total_desc_sizes[n];
    }

    handles.set_layout = &set_layouts_[set_layout_index];
    handles.base_resource_addresses.resize(set_layout.dynamic_resource_count, 0);
    return true;
}

namespace {
template<typename Ty, std::size_t N, std::size_t... Indices>
auto makeHeapsArray(const std::array<Ty, N>& heaps, std::index_sequence<Indices...>) {
    return std::array{heaps[Indices].heap.get()...};
}
}  // namespace

void PipelineLayout::bindRootSignature(ID3D12GraphicsCommandList* command_list) {
    const auto heaps = makeHeapsArray(heaps_, std::make_index_sequence<std::size_t(HeapType::TOTAL_COUNT)>{});
    command_list->SetDescriptorHeaps(UINT(heaps.size()), heaps.data());
    command_list->SetGraphicsRootSignature(root_signature_.get());
}

void PipelineLayout::bindRootDescriptorTables(ID3D12GraphicsCommandList* command_list, std::uint32_t set_index,
                                              DescriptorSet& desc_set, std::span<const std::uint32_t> offsets) {
    const DescriptorSetHandles& handles = desc_set.getDescriptorHandles();
    const auto* binding = desc_set.getLayout().getParamBindings(*handles.set_layout);
    const auto& set_layout = set_layouts_[set_index];
    for (std::uint32_t index = set_layout.first_param_index;
         index < set_layout.first_param_index + set_layout.param_count; ++index, ++binding) {
        if (binding->isDynamic()) {
            const std::uint32_t dynamic_resource_index = binding->getDynamicResourceIndex();
            const D3D12_GPU_VIRTUAL_ADDRESS address = handles.base_resource_addresses[dynamic_resource_index] +
                                                      offsets[dynamic_resource_index];
            switch (binding->getBindingType()) {
                case BindingType::CONSTANT_BUFFER: {
                    command_list->SetGraphicsRootConstantBufferView(index, address);
                } break;
                case BindingType::SHADER_RESOURCE: {
                    command_list->SetGraphicsRootShaderResourceView(index, address);
                } break;
                case BindingType::UNORDERED_ACCESS: {
                    command_list->SetGraphicsRootUnorderedAccessView(index, address);
                } break;
                default: break;
            }
        } else {
            command_list->SetGraphicsRootDescriptorTable(
                index, handles.base_gpu_handles[unsigned(binding->getHeapType())] + binding->getOffset());
        }
    }
}

//@{ IPipelineLayout

util::ref_ptr<IDescriptorSet> PipelineLayout::createDescriptorSet(std::uint32_t set_layout_index) {
    auto descriptor_set = util::make_new<DescriptorSet>(*device_, *this);
    if (!descriptor_set->create(set_layout_index)) { return nullptr; }
    return std::move(descriptor_set);
}

void PipelineLayout::resetDescriptorAllocator() {
    for (auto& heap : heaps_) { heap.used_size = 0; }
}

//@}
