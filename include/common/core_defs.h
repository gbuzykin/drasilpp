#pragma once

#include <cstdint>
#include <limits>

#if defined(WIN32)
#    define DRASILPP_ENTRY_EXPORT extern "C" __declspec(dllexport)
#elif defined(__linux__)
#    define DRASILPP_ENTRY_EXPORT extern "C" __attribute__((visibility("default")))
#endif

namespace drasilpp {

constexpr std::uint32_t INVALID_UINT32_VALUE = std::numeric_limits<std::uint32_t>::max();

template<typename Ty>
const Ty* constAddressOf(Ty&& o) {
    return &static_cast<const Ty&>(o);
}

}  // namespace drasilpp
