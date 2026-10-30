#pragma once

#include <D3D12MemAlloc.h>  // NOLINT
#include <d3d12.h>

#include <cstdint>

#ifdef min
#    undef min
#endif

#ifdef max
#    undef max
#endif

namespace drasilpp::rel::d3d12 {

inline D3D12_CPU_DESCRIPTOR_HANDLE& operator+=(D3D12_CPU_DESCRIPTOR_HANDLE& handle, std::uint32_t offset) {
    handle.ptr += offset;
    return handle;
}

inline D3D12_CPU_DESCRIPTOR_HANDLE operator+(D3D12_CPU_DESCRIPTOR_HANDLE handle, std::uint32_t offset) {
    handle.ptr += offset;
    return handle;
}

inline D3D12_GPU_DESCRIPTOR_HANDLE& operator+=(D3D12_GPU_DESCRIPTOR_HANDLE& handle, std::uint32_t offset) {
    handle.ptr += offset;
    return handle;
}

inline D3D12_GPU_DESCRIPTOR_HANDLE operator+(D3D12_GPU_DESCRIPTOR_HANDLE handle, std::uint32_t offset) {
    handle.ptr += offset;
    return handle;
}

class Buffer;
class DescriptorSet;
class DevQueue;
class Device;
class Fence;
class FrameImageProvider;
class PhysicalDevice;
class PipelineLayout;
class Pipeline;
class RenderTarget;
class RenderingDriver;
class Sampler;
class ShaderModule;
class Surface;
class SwapChain;
class Texture;

}  // namespace drasilpp::rel::d3d12
