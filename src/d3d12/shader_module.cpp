#include "shader_module.h"

#include "device.h"

using namespace drasilpp;
using namespace drasilpp::rel;
using namespace drasilpp::rel::d3d12;

// --------------------------------------------------------
// ShaderModule class implementation

ShaderModule::ShaderModule(Device& device) : device_(util::not_null{&device}) {}

ShaderModule::~ShaderModule() {}

//@{ IShaderModule

//@}
