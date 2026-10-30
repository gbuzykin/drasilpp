#pragma once

#include "d3d12_api.h"

#include "interfaces/i_rendering_driver.h"

namespace drasilpp::rel::d3d12 {

class Sampler final : public ISampler {
 public:
    explicit Sampler(std::shared_ptr<Device> device);
    ~Sampler() override;

    bool create(const SamplerDesc& desc);

    const D3D12_SAMPLER_DESC& getSamplerDesc() { return desc_; }

    //@{ ISampler
    //@}

 private:
    std::shared_ptr<Device> device_;
    D3D12_SAMPLER_DESC desc_;
};

}  // namespace drasilpp::rel::d3d12
