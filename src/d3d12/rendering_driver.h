#pragma once

#include "d3d12_api.h"

#include "interfaces/i_rendering_driver.h"
#include "rel/hlsl_compiler.h"
#include "util/ref_ptr.h"

#include <string>
#include <vector>

namespace drasilpp::rel::d3d12 {

class PhysicalDevice {
 public:
    explicit PhysicalDevice(util::ref_ptr<IDXGIAdapter> adapter);
    PhysicalDevice(const PhysicalDevice&) = delete;
    PhysicalDevice& operator=(const PhysicalDevice&) = delete;

    const std::string& getName() const { return name_; }

    bool loadDescription();

    IDXGIAdapter* getAdapter() { return adapter_.get(); }

 private:
    util::ref_ptr<IDXGIAdapter> adapter_;
    DXGI_ADAPTER_DESC adapter_desc_{};
    std::string name_;
};

class RenderingDriver final : public std::enable_shared_from_this<RenderingDriver>, public IRenderingDriver {
 public:
    RenderingDriver();
    ~RenderingDriver() override;

    IDXGIFactory4* getDXGIFactory() { return dxgi_factory_.get(); }

    //@{ IRenderingDriver
    bool init(const uxs::db::value& app_info) override;
    std::uint32_t getPhysicalDeviceCount() const override;
    const char* getPhysicalDeviceName(std::uint32_t device_index) const override;
    bool isSuitablePhysicalDevice(std::uint32_t device_index, const uxs::db::value& caps) const override;
    DataBlob compileShader(const DataBlob& source_text, const uxs::db::value& args, DataBlob& compiler_output) override;
    std::shared_ptr<ISurface> createSurface(const WindowDescriptor& win_desc) override;
    std::shared_ptr<IDevice> createDevice(std::uint32_t device_index, const uxs::db::value& caps) override;
    std::shared_ptr<ISwapChain> createSwapChain(const std::shared_ptr<IDevice>& device,
                                                const std::shared_ptr<ISurface>& surface,
                                                const uxs::db::value& opts) override;
    std::shared_ptr<IShaderModule> createShaderModule(const std::shared_ptr<IDevice>& device,
                                                      DataBlob bytecode) override;
    std::shared_ptr<IPipelineLayout> createPipelineLayout(const std::shared_ptr<IDevice>& device,
                                                          const uxs::db::value& config) override;
    std::shared_ptr<IPipeline> createPipeline(const std::shared_ptr<IDevice>& device,
                                              const std::shared_ptr<IRenderTarget>& render_target,
                                              const std::shared_ptr<IPipelineLayout>& pipeline_layout,
                                              std::span<IShaderModule* const> shader_modules,
                                              const uxs::db::value& config) override;
    std::shared_ptr<IBuffer> createBuffer(const std::shared_ptr<IDevice>& device, BufferType type,
                                          std::uint64_t size) override;
    std::shared_ptr<ITexture> createTexture(const std::shared_ptr<IDevice>& device, const TextureDesc& desc) override;
    std::shared_ptr<ISampler> createSampler(const std::shared_ptr<IDevice>& device, const SamplerDesc& desc) override;
    std::shared_ptr<IDescriptorSet> createDescriptorSet(const std::shared_ptr<IDevice>& device,
                                                        const std::shared_ptr<IPipelineLayout>& pipeline_layout,
                                                        std::uint32_t set_layout_index) override;
    std::shared_ptr<IRenderTarget> createTextureRenderTarget(const std::shared_ptr<IDevice>& device,
                                                             const std::shared_ptr<ITexture>& texture,
                                                             const uxs::db::value& opts) override;
    std::shared_ptr<IRenderTarget> createSwapChainRenderTarget(const std::shared_ptr<IDevice>& device,
                                                               const std::shared_ptr<ISwapChain>& swap_chain,
                                                               const uxs::db::value& opts) override;
    //@}

 private:
    util::ref_ptr<IDXGIFactory4> dxgi_factory_;
    std::vector<std::unique_ptr<PhysicalDevice>> physical_devices_;
    HlslCompiler hlsl_compiler_;

    bool loadPhysicalDeviceList();
};

}  // namespace drasilpp::rel::d3d12
