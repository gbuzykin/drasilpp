#include "descriptor_set.h"

#include "buffer.h"
#include "device.h"
#include "sampler.h"
#include "texture.h"

using namespace drasilpp;
using namespace drasilpp::rel;
using namespace drasilpp::rel::d3d12;

// --------------------------------------------------------
// DescriptorSet class implementation

DescriptorSet::DescriptorSet(std::shared_ptr<Device> device, std::shared_ptr<PipelineLayout> pipeline_layout)
    : device_(std::move(device)), pipeline_layout_(std::move(pipeline_layout)) {}

DescriptorSet::~DescriptorSet() {}

//@{ IDescriptorSet

void DescriptorSet::updateSamplerDescriptor(ISampler& sampler, std::uint32_t slot) {
    const auto& layout = *handles_.set_layout;
    const auto& sampler_desc = static_cast<Sampler&>(sampler).getSamplerDesc();
    const auto& binding = getLayout().getSlotBinding(layout, BindingType::SAMPLER, slot);
    device_->getD3D12Device()->CreateSampler(
        &sampler_desc, handles_.base_cpu_handles[unsigned(HeapType::SAMPLER)] + binding.getOffset());
}

void DescriptorSet::updateCombinedTextureSamplerDescriptor(ITexture& texture, ISampler& sampler, std::uint32_t slot,
                                                           std::uint32_t sampler_slot) {
    const auto& layout = *handles_.set_layout;
    const auto& srv_desc = static_cast<Texture&>(texture).getShaderResourceViewDesc();
    const auto& sampler_desc = static_cast<Sampler&>(sampler).getSamplerDesc();
    const auto& srv_binding = getLayout().getSlotBinding(layout, BindingType::SHADER_RESOURCE, slot);
    const auto& sampler_binding = getLayout().getSlotBinding(layout, BindingType::SAMPLER, sampler_slot);
    device_->getD3D12Device()->CreateShaderResourceView(
        static_cast<Texture&>(texture).getResource(), &srv_desc,
        handles_.base_cpu_handles[unsigned(HeapType::CBV_SRV_UAV)] + srv_binding.getOffset());
    device_->getD3D12Device()->CreateSampler(
        &sampler_desc, handles_.base_cpu_handles[unsigned(HeapType::SAMPLER)] + sampler_binding.getOffset());
}

void DescriptorSet::updateShaderResourceDescriptor(ITexture& texture, std::uint32_t slot) {
    const auto& layout = *handles_.set_layout;
    const auto& srv_desc = static_cast<Texture&>(texture).getShaderResourceViewDesc();
    const auto& binding = getLayout().getSlotBinding(layout, BindingType::SHADER_RESOURCE, slot);
    device_->getD3D12Device()->CreateShaderResourceView(
        static_cast<Texture&>(texture).getResource(), &srv_desc,
        handles_.base_cpu_handles[unsigned(HeapType::CBV_SRV_UAV)] + binding.getOffset());
}

void DescriptorSet::updateConstantBufferDescriptor(IBuffer& buffer, std::uint64_t offset, std::uint64_t size,
                                                   std::uint32_t slot) {
    const auto& layout = *handles_.set_layout;
    const auto& cbv_desc = static_cast<Buffer&>(buffer).getConstantBufferViewDesc(offset);
    const auto& binding = getLayout().getSlotBinding(layout, BindingType::CONSTANT_BUFFER, slot);
    if (binding.isDynamic()) {
        handles_.base_resource_addresses[binding.getDynamicResourceIndex()] = cbv_desc.BufferLocation;
    } else {
        device_->getD3D12Device()->CreateConstantBufferView(
            &cbv_desc, handles_.base_cpu_handles[unsigned(HeapType::CBV_SRV_UAV)] + binding.getOffset());
    }
}

//@}
