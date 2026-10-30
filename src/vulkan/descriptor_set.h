#pragma once

#include "pipeline_layout.h"

namespace drasilpp::rel::vulkan {

class DescriptorSet final : public IDescriptorSet {
 public:
    DescriptorSet(std::shared_ptr<Device> device, std::shared_ptr<PipelineLayout> pipeline_layout);
    ~DescriptorSet() override;

    bool create(std::uint32_t set_layout_index) {
        return pipeline_layout_->obtainDescriptorSet(set_layout_index, handle_);
    }

    VkDescriptorSet getHandle() { return handle_.handle; }
    PipelineLayout& getLayout() { return *pipeline_layout_; }

    //@{ IDescriptorSet
    void updateSamplerDescriptor(ISampler&, std::uint32_t slot) override;
    void updateCombinedTextureSamplerDescriptor(ITexture& texture, ISampler& sampler, std::uint32_t slot,
                                                std::uint32_t sampler_slot) override;
    void updateShaderResourceDescriptor(ITexture& texture, std::uint32_t slot) override;
    void updateConstantBufferDescriptor(IBuffer& buffer, std::uint64_t offset, std::uint64_t size,
                                        std::uint32_t slot) override;
    //@}

 private:
    std::shared_ptr<Device> device_;
    std::shared_ptr<PipelineLayout> pipeline_layout_;
    PipelineLayout::DescriptorSetHandle handle_{};

    void writeImageDescriptorSet(PipelineLayout::Binding binding, std::span<const VkDescriptorImageInfo> image_infos);
    void writeBufferDescriptorSet(PipelineLayout::Binding binding, std::span<const VkDescriptorBufferInfo> buffer_infos);
};

}  // namespace drasilpp::rel::vulkan
