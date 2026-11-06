#include "shader_module.h"  // IWYU pragma: keep

#include "device.h"

using namespace drasilpp;
using namespace drasilpp::rel;
using namespace drasilpp::rel::d3d12;

// --------------------------------------------------------
// ShaderModule class implementation

ShaderModule::ShaderModule(std::shared_ptr<Device> device) : device_(std::move(device)) {}

ShaderModule::~ShaderModule() {}

//@{ IShaderModule

//@}
