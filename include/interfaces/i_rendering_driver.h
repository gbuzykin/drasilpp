#pragma once

#include "rel/data_blob.h"
#include "rel/structs.h"
#include "rel/win_desc.h"

#include <uxs/db/value.h>

#if defined(WIN32)
#    define DRASILPP_ENTRY_EXPORT extern "C" __declspec(dllexport)
#elif defined(__linux__)
#    define DRASILPP_ENTRY_EXPORT extern "C" __attribute__((visibility("default")))
#endif

#define DRASILPP_REGISTER_RENDERING_DRIVER(name, driver_class) \
    constexpr drasilpp::rel::DriverDesc g_rendering_driver_descriptor{ \
        name, []() -> std::shared_ptr<drasilpp::rel::IRenderingDriver> { return std::make_shared<driver_class>(); }}; \
    DRASILPP_ENTRY_EXPORT const drasilpp::rel::DriverDesc* drasilppGetRenderingDriverDescriptor() { \
        return &g_rendering_driver_descriptor; \
    } \
    static_assert(true)

namespace drasilpp::rel {

enum class RenderTargetResult {
    SUCCESS = 0,
    SUBOPTIMAL,
    OUT_OF_DATE,
    FAILED,
};

struct ISurface {
    virtual ~ISurface() = default;
};

struct IDevice {
    virtual ~IDevice() = default;
    virtual bool waitDevice() = 0;
};

struct ISwapChain {
    virtual ~ISwapChain() = default;
    virtual bool recreate(const uxs::db::value& opts) = 0;
};

struct IShaderModule {
    virtual ~IShaderModule() = default;
};

struct IPipelineLayout {
    virtual ~IPipelineLayout() = default;
    virtual void resetDescriptorAllocator() = 0;
};

struct IPipeline {
    virtual ~IPipeline() = default;
};

struct IBuffer {
    virtual ~IBuffer() = default;
    virtual bool updateBuffer(std::span<const std::uint8_t> data, std::uint64_t offset) = 0;
};

struct ITexture {
    virtual ~ITexture() = default;
    virtual bool updateTexture(const std::uint8_t* data, std::uint32_t first_subresource,
                               std::span<const UpdateTextureDesc> update_subresource_descs) = 0;
};

struct ISampler {
    virtual ~ISampler() = default;
};

struct IDescriptorSet {
    virtual ~IDescriptorSet() = default;
    virtual void updateSamplerDescriptor(ISampler& sampler, std::uint32_t slot) = 0;
    virtual void updateCombinedTextureSamplerDescriptor(ITexture& texture, ISampler& sampler, std::uint32_t slot,
                                                        std::uint32_t sampler_slot) = 0;
    virtual void updateShaderResourceDescriptor(ITexture& texture, std::uint32_t slot) = 0;
    virtual void updateConstantBufferDescriptor(IBuffer& buffer, std::uint64_t offset, std::uint64_t size,
                                                std::uint32_t slot) = 0;
};

struct IRenderTarget {
    virtual ~IRenderTarget() = default;
    virtual Extent2u getImageExtent() const = 0;
    virtual std::uint32_t getFifCount() const = 0;
    virtual bool isInvertedNdcY() const = 0;
    virtual RenderTargetResult beginRenderTarget(const Color4f& clear_color, float depth, std::uint32_t stencil,
                                                 IPipeline& pipeline) = 0;
    virtual bool endRenderTarget() = 0;
    virtual void setViewport(const Rect& rect, float z_near, float z_far) = 0;
    virtual void setScissor(const Rect& rect) = 0;
    virtual void bindPipeline(IPipeline& pipeline) = 0;
    virtual void bindVertexBuffer(IBuffer& buffer, std::uint32_t slot, std::uint32_t stride, std::uint32_t offset) = 0;
    virtual void bindDescriptorSet(IDescriptorSet& descriptor_set, std::uint32_t set_index) = 0;
    virtual void bindDescriptorSetDynamic(IDescriptorSet& descriptor_set, std::uint32_t set_index,
                                          std::span<const std::uint32_t> offsets) = 0;
    virtual void setPrimitiveTopology(PrimitiveTopology topology) = 0;
    virtual void drawGeometry(std::uint32_t vertex_count, std::uint32_t instance_count, std::uint32_t first_vertex,
                              std::uint32_t first_instance) = 0;
};

struct IRenderingDriver {
    virtual ~IRenderingDriver() = default;
    virtual bool init(const uxs::db::value& app_info) = 0;
    virtual std::uint32_t getPhysicalDeviceCount() const = 0;
    virtual const char* getPhysicalDeviceName(std::uint32_t device_index) const = 0;
    virtual bool isSuitablePhysicalDevice(std::uint32_t device_index, const uxs::db::value& caps) const = 0;
    virtual DataBlob compileShader(const DataBlob& source_text, const uxs::db::value& args,
                                   DataBlob& compiler_output) = 0;
    virtual std::shared_ptr<ISurface> createSurface(const WindowDescriptor& win_desc) = 0;
    virtual std::shared_ptr<IDevice> createDevice(std::uint32_t device_index, const uxs::db::value& caps) = 0;
    virtual std::shared_ptr<ISwapChain> createSwapChain(const std::shared_ptr<IDevice>& device,
                                                        const std::shared_ptr<ISurface>& surface,
                                                        const uxs::db::value& opts) = 0;
    virtual std::shared_ptr<IShaderModule> createShaderModule(const std::shared_ptr<IDevice>& device,
                                                              DataBlob bytecode) = 0;
    virtual std::shared_ptr<IPipelineLayout> createPipelineLayout(const std::shared_ptr<IDevice>& device,
                                                                  const uxs::db::value& config) = 0;
    virtual std::shared_ptr<IPipeline> createPipeline(const std::shared_ptr<IDevice>& device,
                                                      const std::shared_ptr<IRenderTarget>& render_target,
                                                      const std::shared_ptr<IPipelineLayout>& pipeline_layout,
                                                      std::span<IShaderModule* const> shader_modules,
                                                      const uxs::db::value& config) = 0;
    virtual std::shared_ptr<IBuffer> createBuffer(const std::shared_ptr<IDevice>& device, BufferType type,
                                                  std::uint64_t size) = 0;
    virtual std::shared_ptr<ITexture> createTexture(const std::shared_ptr<IDevice>& device, const TextureDesc& desc) = 0;
    virtual std::shared_ptr<ISampler> createSampler(const std::shared_ptr<IDevice>& device, const SamplerDesc& desc) = 0;
    virtual std::shared_ptr<IDescriptorSet> createDescriptorSet(const std::shared_ptr<IDevice>& device,
                                                                const std::shared_ptr<IPipelineLayout>& pipeline_layout,
                                                                std::uint32_t set_layout_index) = 0;
    virtual std::shared_ptr<IRenderTarget> createTextureRenderTarget(const std::shared_ptr<IDevice>& device,
                                                                     const std::shared_ptr<ITexture>& texture,
                                                                     const uxs::db::value& opts) = 0;
    virtual std::shared_ptr<IRenderTarget> createSwapChainRenderTarget(const std::shared_ptr<IDevice>& device,
                                                                       const std::shared_ptr<ISwapChain>& swap_chain,
                                                                       const uxs::db::value& opts) = 0;
};

// Registered rendering driver descriptor
struct DriverDesc {
    std::string_view name;
    std::shared_ptr<IRenderingDriver> (*create_func)();
};

using GetDriverDescriptorFuncPtr = const DriverDesc* (*)();

}  // namespace drasilpp::rel
