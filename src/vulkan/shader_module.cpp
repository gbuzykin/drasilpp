#include "shader_module.h"

#include "device.h"
#include "vulkan_logger.h"

#include <utility>

using namespace drasilpp;
using namespace drasilpp::rel;
using namespace drasilpp::rel::vulkan;

// --------------------------------------------------------
// ShaderModule class implementation

ShaderModule::ShaderModule(std::shared_ptr<Device> device) : device_(std::move(device)) {}

ShaderModule::~ShaderModule() { device_->vkDestroyShaderModule(shader_module_, nullptr); }

bool ShaderModule::create(const DataBlob& bytecode) {
    const VkShaderModuleCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = bytecode.getSize(),
        .pCode = reinterpret_cast<const std::uint32_t*>(bytecode.getData()),
    };

    VkResult result = device_->vkCreateShaderModule(&create_info, nullptr, &shader_module_);
    if (result != VK_SUCCESS) {
        logError(LOG_VK "couldn't create shader module: {}", result);
        return false;
    }

    return true;
}

//@{ IShaderModule

//@}
