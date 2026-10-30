#pragma once

#include "vulkan_api.h"

#include "interfaces/i_rendering_driver.h"
#include "rel/hlsl_compiler.h"

#include <vector>

namespace drasilpp::rel::vulkan {

class RenderingDriver final : public std::enable_shared_from_this<RenderingDriver>, public IRenderingDriver {
 public:
    RenderingDriver();
    ~RenderingDriver() override;

#define INSTANCE_LEVEL_VK_FUNCTION(name) \
    template<typename... Args> \
    auto name(Args&&... args) { \
        return vk_funcs_.name(instance_, std::forward<Args>(args)...); \
    }

#define INSTANCE_LEVEL_VK_FUNCTION_FROM_EXTENSION(name, extension) \
    template<typename... Args> \
    auto name(Args&&... args) { \
        return vk_funcs_.name(instance_, std::forward<Args>(args)...); \
    }

#include "vulkan_function_list.inl"

    const InstanceVkFuncTable& getVkFuncs() { return vk_funcs_; }

    bool isExtensionSupported(const char* extension) const;
    std::span<Surface*> getSurfaces() { return surfaces_; }
    void removeSurface(Surface* surface) { std::erase(surfaces_, surface); }

    VkInstance getHandle() { return instance_; }

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
    void* vulkan_library_ = nullptr;
    InstanceVkFuncTable vk_funcs_;
    VkInstance instance_{VK_NULL_HANDLE};
    std::vector<VkExtensionProperties> extensions_;
    std::vector<std::unique_ptr<PhysicalDevice>> physical_devices_;
    std::vector<Surface*> surfaces_;
    HlslCompiler hlsl_compiler_;

    bool loadVulkanLoaderLibrary();
    bool loadExtensionProperties();
    bool createInstance(const uxs::db::value& app_info);
    bool loadPhysicalDeviceList();
};

class PhysicalDevice {
 public:
    explicit PhysicalDevice(RenderingDriver& instance, VkPhysicalDevice physical_device);
    PhysicalDevice(const PhysicalDevice&) = delete;
    PhysicalDevice& operator=(const PhysicalDevice&) = delete;

    bool isExtensionSupported(const char* extension) const;
    const VkPhysicalDeviceProperties& getProperties() const { return properties_; }
    const VkPhysicalDeviceFeatures& getFeatures() const { return features_; }
    const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const { return memory_properties_; }
    std::span<const VkQueueFamilyProperties> getQueueFamilies() const { return queue_families_; }
    std::uint32_t findSuitableQueueFamily(VkQueueFlags flags, std::uint32_t n = 0) const;
    bool isSuitableDevice(const uxs::db::value& caps) const;

    bool loadExtensionProperties();
    bool loadFeaturesAndProperties();

    VkPhysicalDevice getHandle() { return physical_device_; }

#define INSTANCE_LEVEL_VK_FUNCTION_PHYDEV(name) \
    template<typename... Args> \
    auto name(Args&&... args) { \
        return instance_.getVkFuncs().name(physical_device_, std::forward<Args>(args)...); \
    }

#define INSTANCE_LEVEL_VK_FUNCTION_FROM_EXTENSION_PHYDEV(name, extension) \
    template<typename... Args> \
    auto name(Args&&... args) { \
        return instance_.getVkFuncs().name(physical_device_, std::forward<Args>(args)...); \
    }

#include "vulkan_function_list.inl"

 private:
    RenderingDriver& instance_;
    VkPhysicalDevice physical_device_;
    std::vector<VkExtensionProperties> extensions_;
    VkPhysicalDeviceProperties properties_{};
    VkPhysicalDeviceFeatures features_{};
    VkPhysicalDeviceMemoryProperties memory_properties_{};
    std::vector<VkQueueFamilyProperties> queue_families_;
};

}  // namespace drasilpp::rel::vulkan
