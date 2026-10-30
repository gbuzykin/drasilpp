#pragma once

#include "frame_image_provider.h"

#include "util/ref_ptr.h"

#include <uxs/dynarray.h>

namespace drasilpp::rel::d3d12 {

class SwapChain final : public FrameImageProvider, public ISwapChain {
 public:
    SwapChain(std::shared_ptr<Device> device, std::shared_ptr<Surface> surface);
    ~SwapChain() override;

    bool create(const uxs::db::value& opts);

    //@{ FrameImageProvider
    ID3D12Resource* getImageResource(std::uint32_t image_index) override { return images_[image_index].get(); }
    std::uint32_t getImageCount() const override { return std::uint32_t(images_.size()); }
    std::uint32_t getFifCount() const override { return SWAP_CHAIN_BUFFER_COUNT; }
    Format getImageFormat() const override { return BACK_BUFFER_FORMAT; }
    Extent2u getImageExtent() const override { return image_extent_; }
    void imageBarrierBefore(ID3D12GraphicsCommandList* command_list, std::uint32_t image_index) override;
    void imageBarrierAfter(ID3D12GraphicsCommandList* command_list, std::uint32_t image_index) override;
    RenderTargetResult presentImage() override;
    void setRenderTarget(RenderTarget* render_target) override;
    void removeRenderTarget(RenderTarget* render_target) override;
    //@}

    //@{ ISwapChain
    bool recreate(const uxs::db::value& opts) override;
    //@}

 private:
    std::shared_ptr<Device> device_;
    std::shared_ptr<Surface> surface_;
    util::ref_ptr<IDXGISwapChain> swap_chain_;

    static constexpr Format BACK_BUFFER_FORMAT = Format::R8G8B8A8_UNORM;
    static constexpr std::uint32_t SWAP_CHAIN_BUFFER_COUNT = 3;

    Extent2u image_extent_{};
    uxs::inline_dynarray<util::ref_ptr<ID3D12Resource>, 3> images_;
    RenderTarget* render_target_ = nullptr;

    bool recreateSwapChainResources(const uxs::db::value& opts);
};

}  // namespace drasilpp::rel::d3d12
