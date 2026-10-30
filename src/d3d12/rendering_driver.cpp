#include "rendering_driver.h"

#include "buffer.h"
#include "d3d12_logger.h"
#include "descriptor_set.h"
#include "device.h"
#include "pipeline.h"
#include "render_target.h"
#include "sampler.h"
#include "shader_module.h"
#include "surface.h"
#include "swap_chain.h"
#include "texture.h"

#include <uxs/string_utf.h>

using namespace drasilpp;
using namespace drasilpp::rel;
using namespace drasilpp::rel::d3d12;

// --------------------------------------------------------
// RenderingDriver class implementation

RenderingDriver::RenderingDriver() {}

RenderingDriver::~RenderingDriver() { logDebug(LOG_D3D12 "destroy RenderingDriver"); }

//@{ IRenderingDriver

bool RenderingDriver::init(const uxs::db::value& app_info) {
#if !defined(NDEBUG)
    {  // Enable the D3D12 debug layer.
        util::ref_ptr<ID3D12Debug> debug_controller;
        HRESULT result = ::D3D12GetDebugInterface(IID_PPV_ARGS(debug_controller.reset_and_get_address()));
        if (result != S_OK) {
            logError(LOG_D3D12 "couldn't get debug interface: {}", D3D12Result{result});
            return false;
        }
        debug_controller->EnableDebugLayer();
    }
#endif

    HRESULT result = CreateDXGIFactory1(IID_PPV_ARGS(dxgi_factory_.reset_and_get_address()));
    if (result != S_OK) {
        logError(LOG_D3D12 "couldn't create DXGI factory: {}", D3D12Result{result});
        return false;
    }

    if (!loadPhysicalDeviceList()) { return false; }

    uxs::db::value platform_args;
    platform_args["args"] = uxs::db::value(uxs::db::array_tag, {"-O3", "-Dbinding(n)=", "-Dlocation(n)="});

    hlsl_compiler_.setPlatformArgs(platform_args);

    return true;
}

std::uint32_t RenderingDriver::getPhysicalDeviceCount() const { return std::uint32_t(physical_devices_.size()); }

const char* RenderingDriver::getPhysicalDeviceName(std::uint32_t device_index) const {
    return physical_devices_[device_index]->getName().c_str();
}

bool RenderingDriver::isSuitablePhysicalDevice(std::uint32_t device_index, const uxs::db::value& caps) const {
    return true;
}

DataBlob RenderingDriver::compileShader(const DataBlob& source_text, const uxs::db::value& args,
                                        DataBlob& compiler_output) {
    return hlsl_compiler_.compileShader(source_text, args, compiler_output);
}

std::shared_ptr<ISurface> RenderingDriver::createSurface(const WindowDescriptor& win_desc) {
    auto surface = std::make_shared<Surface>(shared_from_this());
    if (!surface->create(win_desc)) { return nullptr; }
    return std::move(surface);
}

std::shared_ptr<IDevice> RenderingDriver::createDevice(std::uint32_t device_index, const uxs::db::value& caps) {
    auto& physical_device = *physical_devices_[device_index];
    auto device = std::make_shared<Device>(shared_from_this(), physical_device);
    if (!device->create(caps)) { return nullptr; }
    return std::move(device);
}

std::shared_ptr<ISwapChain> RenderingDriver::createSwapChain(const std::shared_ptr<IDevice>& device,
                                                             const std::shared_ptr<ISurface>& surface,
                                                             const uxs::db::value& opts) {
    auto swap_chain = std::make_shared<SwapChain>(std::static_pointer_cast<Device>(device),
                                                  std::static_pointer_cast<Surface>(surface));
    if (!swap_chain->create(opts)) { return nullptr; }
    return std::move(swap_chain);
}

std::shared_ptr<IShaderModule> RenderingDriver::createShaderModule(const std::shared_ptr<IDevice>& device,
                                                                   DataBlob bytecode) {
    auto shader_module = std::make_shared<ShaderModule>(std::static_pointer_cast<Device>(device));
    if (!shader_module->create(std::move(bytecode))) { return nullptr; }
    return std::move(shader_module);
}

std::shared_ptr<IPipelineLayout> RenderingDriver::createPipelineLayout(const std::shared_ptr<IDevice>& device,
                                                                       const uxs::db::value& config) {
    auto pipeline_layout = std::make_shared<PipelineLayout>(std::static_pointer_cast<Device>(device));
    if (!pipeline_layout->create(config)) { return nullptr; }
    return std::move(pipeline_layout);
}

std::shared_ptr<IPipeline> RenderingDriver::createPipeline(const std::shared_ptr<IDevice>& device,
                                                           const std::shared_ptr<IRenderTarget>& render_target,
                                                           const std::shared_ptr<IPipelineLayout>& pipeline_layout,
                                                           std::span<IShaderModule* const> shader_modules,
                                                           const uxs::db::value& config) {
    auto pipeline = std::make_shared<Pipeline>(std::static_pointer_cast<Device>(device),
                                               std::static_pointer_cast<RenderTarget>(render_target),
                                               std::static_pointer_cast<PipelineLayout>(pipeline_layout));
    if (!pipeline->create(shader_modules, config)) { return nullptr; }
    return std::move(pipeline);
}

std::shared_ptr<IBuffer> RenderingDriver::createBuffer(const std::shared_ptr<IDevice>& device, BufferType type,
                                                       std::uint64_t size) {
    auto buffer = std::make_shared<Buffer>(std::static_pointer_cast<Device>(device));
    if (!buffer->create(type, size)) { return nullptr; }
    return std::move(buffer);
}

std::shared_ptr<ITexture> RenderingDriver::createTexture(const std::shared_ptr<IDevice>& device,
                                                         const TextureDesc& desc) {
    auto texture = std::make_shared<Texture>(std::static_pointer_cast<Device>(device));
    if (!texture->create(desc)) { return nullptr; }
    return std::move(texture);
}

std::shared_ptr<ISampler> RenderingDriver::createSampler(const std::shared_ptr<IDevice>& device,
                                                         const SamplerDesc& desc) {
    auto sampler = std::make_shared<Sampler>(std::static_pointer_cast<Device>(device));
    if (!sampler->create(desc)) { return nullptr; }
    return std::move(sampler);
}

std::shared_ptr<IDescriptorSet> RenderingDriver::createDescriptorSet(
    const std::shared_ptr<IDevice>& device, const std::shared_ptr<IPipelineLayout>& pipeline_layout,
    std::uint32_t set_layout_index) {
    auto descriptor_set = std::make_shared<DescriptorSet>(std::static_pointer_cast<Device>(device),
                                                          std::static_pointer_cast<PipelineLayout>(pipeline_layout));
    if (!descriptor_set->create(set_layout_index)) { return nullptr; }
    return std::move(descriptor_set);
}

std::shared_ptr<IRenderTarget> RenderingDriver::createTextureRenderTarget(const std::shared_ptr<IDevice>& device,
                                                                          const std::shared_ptr<ITexture>& texture,
                                                                          const uxs::db::value& opts) {
    auto render_target = std::make_shared<RenderTarget>(std::static_pointer_cast<Device>(device),
                                                        std::static_pointer_cast<Texture>(texture));
    if (!render_target->create(opts)) { return nullptr; }
    return std::move(render_target);
}

std::shared_ptr<IRenderTarget> RenderingDriver::createSwapChainRenderTarget(
    const std::shared_ptr<IDevice>& device, const std::shared_ptr<ISwapChain>& swap_chain, const uxs::db::value& opts) {
    auto render_target = std::make_shared<RenderTarget>(std::static_pointer_cast<Device>(device),
                                                        std::static_pointer_cast<SwapChain>(swap_chain));
    if (!render_target->create(opts)) { return nullptr; }
    return std::move(render_target);
}

//@}

bool RenderingDriver::loadPhysicalDeviceList() {
    util::ref_ptr<IDXGIAdapter> adapter;
    for (UINT i = 0; dxgi_factory_->EnumAdapters(i, adapter.reset_and_get_address()) != DXGI_ERROR_NOT_FOUND; ++i) {
        auto physical_device = std::make_unique<PhysicalDevice>(std::move(adapter));
        if (physical_device->loadDescription()) { physical_devices_.emplace_back(std::move(physical_device)); }
    }
    return true;
}

// --------------------------------------------------------
// PhysicalDevice class implementation

PhysicalDevice::PhysicalDevice(util::ref_ptr<IDXGIAdapter> adapter) : adapter_(std::move(adapter)) {}

bool PhysicalDevice::loadDescription() {
    adapter_->GetDesc(&adapter_desc_);
    name_ = uxs::utf_string_adapter<char>{}(adapter_desc_.Description);
    return true;
}

DRASILPP_REGISTER_RENDERING_DRIVER("D3D12", RenderingDriver);
