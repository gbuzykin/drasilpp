#pragma once

#include "d3d12_api.h"

#include "interfaces/i_rendering_driver.h"
#include "util/ref_ptr.h"

#include <uxs/dynarray.h>

namespace drasilpp::rel::d3d12 {

class Pipeline final : public IPipeline {
 public:
    Pipeline(std::shared_ptr<Device> device, std::shared_ptr<RenderTarget> render_target,
             std::shared_ptr<PipelineLayout> pipeline_layout);
    ~Pipeline() override;

    std::uint32_t getVertexStride(std::uint32_t slot) const { return vertex_strides_[slot]; }

    bool create(std::span<IShaderModule* const> shader_modules, const uxs::db::value& config);

    ID3D12PipelineState* getD3D12PipelineState() { return pipeline_.get(); }
    PipelineLayout& getLayout() { return *pipeline_layout_; }

    //@{ IPipeline
    //@}

 private:
    std::shared_ptr<Device> device_;
    std::shared_ptr<RenderTarget> render_target_;
    std::shared_ptr<PipelineLayout> pipeline_layout_;
    util::ref_ptr<ID3D12PipelineState> pipeline_;

    uxs::inline_dynarray<std::uint32_t> vertex_strides_;
};

}  // namespace drasilpp::rel::d3d12
