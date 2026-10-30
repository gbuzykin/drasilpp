#pragma once

#include "vulkan_api.h"

#include "interfaces/i_rendering_driver.h"

namespace drasilpp::rel::vulkan {

class Sampler final : public ISampler {
 public:
    explicit Sampler(std::shared_ptr<Device> device);
    ~Sampler() override;

    bool create(const SamplerDesc& desc);

    VkSampler getHandle() { return sampler_; }

    //@{ ISampler
    //@}

 private:
    std::shared_ptr<Device> device_;
    VkSampler sampler_{VK_NULL_HANDLE};
};

}  // namespace drasilpp::rel::vulkan
