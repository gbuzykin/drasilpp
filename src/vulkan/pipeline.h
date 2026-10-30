#pragma once

#include "vulkan_api.h"

#include "interfaces/i_rendering_driver.h"

namespace drasilpp::rel::vulkan {

class Pipeline final : public IPipeline {
 public:
    Pipeline(std::shared_ptr<Device> device, std::shared_ptr<RenderTarget> render_target,
             std::shared_ptr<PipelineLayout> pipeline_layout);
    ~Pipeline() override;

    bool create(std::span<IShaderModule* const> shader_modules, const uxs::db::value& config);

    VkPipeline getHandle() { return pipeline_; }
    PipelineLayout& getLayout() { return *pipeline_layout_; }

    //@{ IPipeline
    //@}

 private:
    std::shared_ptr<Device> device_;
    std::shared_ptr<RenderTarget> render_target_;
    std::shared_ptr<PipelineLayout> pipeline_layout_;
    VkPipeline pipeline_{VK_NULL_HANDLE};
};

}  // namespace drasilpp::rel::vulkan
