#pragma once

#include "vulkan_api.h"

#include "interfaces/i_rendering_driver.h"

namespace drasilpp::rel::vulkan {

class Buffer : public IBuffer {
 public:
    explicit Buffer(std::shared_ptr<Device> device);
    ~Buffer() override;

    VkDeviceSize getSize() const { return size_; }
    VkDeviceSize getAlignment() const { return alignment_; }

    bool create(BufferType type, VkDeviceSize size);

    VkBuffer getHandle() { return buffer_; }

    //@{ IBuffer
    bool updateBuffer(std::span<const std::uint8_t> data, std::uint64_t offset) override;
    //@}

 private:
    std::shared_ptr<Device> device_;
    BufferType type_{};
    VkDeviceSize size_ = 0;
    VkDeviceSize alignment_ = 1;
    VkBuffer buffer_{VK_NULL_HANDLE};
    VmaAllocation allocation_{VK_NULL_HANDLE};
};

}  // namespace drasilpp::rel::vulkan
