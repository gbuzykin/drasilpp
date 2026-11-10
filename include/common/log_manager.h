#pragma once

#include "common/config.h"

#include <uxs/format.h>
#include <uxs/format_fs.h>  // IWYU pragma: keep
#include <uxs/io/filebuf.h>
#include <uxs/string_conv.h>

#include <cstdint>
#include <filesystem>
#include <mutex>
#include <string_view>
#include <vector>

namespace drasilpp {

enum class LogLevel : unsigned { LL_CRITICAL = 0, LL_ERROR, LL_WARNING, LL_INFO, LL_DEBUG };

class DRASILPP_COMMON_EXPORT LogManager {
 public:
    struct IListener {
        virtual ~IListener() = default;
        virtual void onLogMessage(std::uint64_t usecs_since_epoch, LogLevel level, std::string_view message) = 0;
    };

    static LogManager& instance();

    void init(std::string_view instance_name);
    unsigned getInstanceIndex() const { return instance_index_; }
    const std::filesystem::path& getLogFilePath() const { return log_file_path_; }
    void setLogLevel(LogLevel level) { output_level_ = level; }
    void logMessage(LogLevel level, std::string_view message);
    void registerListener(IListener* listener);
    void unregisterListener(IListener* listener);

 private:
    LogManager();

    unsigned instance_index_ = 0;
    std::filesystem::path log_file_path_;
    LogLevel output_level_ = LogLevel::LL_INFO;

    std::mutex mtx_;
    uxs::filebuf log_file_;
    std::vector<IListener*> listeners_;
};

inline LogManager& getLogManager() { return LogManager::instance(); }

class LogMessageStream {
 public:
    explicit LogMessageStream(LogLevel level) : level_(level) {}

    LogMessageStream& operator<<(std::string_view s) {
        buf_.append(s.data(), s.size());
        return *this;
    }

    template<uxs::to_string_convertible Ty>
    LogMessageStream& operator<<(const Ty& v) {
        uxs::to_string_append(buf_, v);
        return *this;
    }

    template<typename... Args>
    LogMessageStream& printLn(uxs::format_string<Args...> fmt, const Args&... args) {
        uxs::vformat_append(buf_, fmt.get(), uxs::format_args::make(args...));
        return showLn();
    }

    template<typename... Args>
    LogMessageStream& printNoLn(uxs::format_string<Args...> fmt, const Args&... args) {
        uxs::vformat_append(buf_, fmt.get(), uxs::format_args::make(args...));
        return *this;
    }

    LogMessageStream& showLn() {
        if (buf_.empty()) { return *this; }
        getLogManager().logMessage(level_, std::string_view(buf_.data(), buf_.size()));
        buf_.clear();
        return *this;
    }

    LogMessageStream& operator<<(LogMessageStream& (*fn)(LogMessageStream&)) { return fn(*this); }

 private:
    LogLevel level_;
    uxs::inline_dynbuffer buf_;
};

inline LogMessageStream critical() { return LogMessageStream(LogLevel::LL_CRITICAL); }
inline LogMessageStream error() { return LogMessageStream(LogLevel::LL_ERROR); }
inline LogMessageStream warning() { return LogMessageStream(LogLevel::LL_WARNING); }
inline LogMessageStream info() { return LogMessageStream(LogLevel::LL_INFO); }
inline LogMessageStream debug(unsigned level = 0) {
    return LogMessageStream(LogLevel{unsigned(LogLevel::LL_DEBUG) + level});
}

inline LogMessageStream& showLn(LogMessageStream& os) { return os.showLn(); }

}  // namespace drasilpp
