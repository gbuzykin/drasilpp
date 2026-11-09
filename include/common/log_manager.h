#pragma once

#include "common/config.h"

#include <uxs/format.h>
#include <uxs/format_fs.h>  // IWYU pragma: keep

#include <string_view>

namespace drasilpp {

enum class LogLevel : unsigned { LL_ERROR = 0, LL_WARNING, LL_INFO, LL_DEBUG };

DRASILPP_COMMON_EXPORT void setLogLevel(LogLevel level);

namespace detail {
DRASILPP_COMMON_EXPORT void vprint(LogLevel level, std::string_view fmt, uxs::format_args args);
}

struct Logger {
    LogLevel level;

    template<typename... Args>
    void printLn(uxs::format_string<Args...> fmt, const Args&... args) {
        detail::vprint(level, fmt.get(), uxs::format_args::make(args...));
    }
};

inline Logger error() { return Logger{LogLevel::LL_ERROR}; }
inline Logger warning() { return Logger{LogLevel::LL_WARNING}; }
inline Logger info() { return Logger{LogLevel::LL_INFO}; }
inline Logger debug(unsigned level = 0) { return Logger{LogLevel{unsigned(LogLevel::LL_DEBUG) + level}}; }

}  // namespace drasilpp
