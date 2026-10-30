#pragma once

#include "pipeline_layout.h"

namespace drasilpp::rel::d3d12 {

class DescriptorSet final : public IDescriptorSet {
 public:
    DescriptorSet(std::shared_ptr<Device> device, std::shared_ptr<PipelineLayout> pipeline_layout);
    ~DescriptorSet() override;

    using HeapType = PipelineLayout::HeapType;

    bool create(std::uint32_t set_layout_index) {
        return pipeline_layout_->obtainDescriptorSet(set_layout_index, handles_);
    }

    PipelineLayout& getLayout() { return *pipeline_layout_; }
    const PipelineLayout::DescriptorSetHandles& getDescriptorHandles() { return handles_; }

    //@{ IDescriptorSet
    void updateSamplerDescriptor(ISampler& sampler, std::uint32_t slot) override;
    void updateCombinedTextureSamplerDescriptor(ITexture& texture, ISampler& sampler, std::uint32_t slot,
                                                std::uint32_t sampler_slot) override;
    void updateShaderResourceDescriptor(ITexture& texture, std::uint32_t slot) override;
    void updateConstantBufferDescriptor(IBuffer& buffer, std::uint64_t offset, std::uint64_t size,
                                        std::uint32_t slot) override;
    //@}

 private:
    std::shared_ptr<Device> device_;
    std::shared_ptr<PipelineLayout> pipeline_layout_;
    PipelineLayout::DescriptorSetHandles handles_{};
};

}  // namespace drasilpp::rel::d3d12
