#pragma once

#include "d3d12_api.h"

#include "interfaces/i_rendering_driver.h"

#include <utility>

namespace drasilpp::rel::d3d12 {

class ShaderModule final : public IShaderModule {
 public:
    explicit ShaderModule(std::shared_ptr<Device> device);
    ~ShaderModule() override;

    const DataBlob& getBytecode() const { return bytecode_; }

    bool create(DataBlob bytecode) {
        bytecode_ = std::move(bytecode);
        return true;
    }

    //@{ IShaderModule
    //@}

 private:
    std::shared_ptr<Device> device_;
    DataBlob bytecode_;
};

}  // namespace drasilpp::rel::d3d12
