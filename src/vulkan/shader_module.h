#pragma once

#include "vulkan_api.h"

#include "interfaces/i_rendering_driver.h"

namespace drasilpp::rel::vulkan {

class ShaderModule final : public IShaderModule {
 public:
    explicit ShaderModule(std::shared_ptr<Device> device);
    ~ShaderModule() override;

    bool create(const DataBlob& bytecode);

    VkShaderModule getHandle() { return shader_module_; }

    //@{ IShaderModule
    //@}

 private:
    std::shared_ptr<Device> device_;
    VkShaderModule shader_module_{VK_NULL_HANDLE};
};

}  // namespace drasilpp::rel::vulkan
