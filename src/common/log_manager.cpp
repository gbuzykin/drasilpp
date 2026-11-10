#include "common/log_manager.h"  // IWYU pragma: keep

#include "common/env_manager.h"

#include <uxs/algorithm.h>
#include <uxs/format_chrono.h>  // IWYU pragma: keep
#include <uxs/io/iobuf.h>

#include <chrono>
#include <stdexcept>

using namespace drasilpp;

LogManager::LogManager() {
#if DRASILPP_LOG_DEBUG_LEVEL > 0
    output_level_ = LogLevel{unsigned(LogLevel::LL_DEBUG) + DRASILPP_LOG_DEBUG_LEVEL - 1};
#endif
}

LogManager& LogManager::instance() {
    static LogManager log_manager;
    return log_manager;
}

void LogManager::init(std::string_view instance_name) {
    constexpr unsigned max_instance_index = 100;
    log_file_.close();

    for (instance_index_ = 0; instance_index_ < max_instance_index; ++instance_index_) {
        const auto& local_host = getEnvManager().getLocalHostName();
        const auto file_name = instance_index_ > 0 ?
                                   uxs::format("{}-{}.log", instance_name, local_host) :
                                   uxs::format("{}-{}-{}.log", instance_name, local_host, instance_index_);

        log_file_path_ = (getEnvManager().getHomePath() / file_name).lexically_normal();

        if (log_file_.open(log_file_path_.c_str(), "w")) { break; }
    }

    if (instance_index_ == max_instance_index) { log_file_path_.clear(); }

    uxs::println("----------------------------------------------------------------------");
    uxs::println("Welcome to {} log manager!", instance_name);

    if (log_file_) { uxs::println("All messages will be written to log file: {}", log_file_path_); }
}

void LogManager::logMessage(LogLevel level, std::string_view message) {
    if (level > output_level_) { return; }

    using namespace std::chrono;
    const auto current_time = system_clock::now() + seconds{getEnvManager().getCurrentTimeZoneOffset()};

    std::string_view color, prefix;
    switch (level) {
        case LogLevel::LL_CRITICAL: color = "\033[1;31m", prefix = "Critical Error: "; break;
        case LogLevel::LL_ERROR: color = "\033[0;31m", prefix = "Error: "; break;
        case LogLevel::LL_WARNING: color = "\033[0;35m", prefix = "Warning: "; break;
        case LogLevel::LL_INFO: color = "\033[0m"; break;
        default: color = "\033[0;33m", prefix = "Debug: "; break;
    }

    const auto current_time_sec = floor<seconds>(current_time);
    const std::uint64_t usecs_since_epoch = duration_cast<microseconds>(current_time.time_since_epoch()).count();

    std::lock_guard lk(mtx_);

    for (auto listener : listeners_) { listener->onLogMessage(usecs_since_epoch, level, message); }

    if (log_file_) { uxs::println(log_file_, "{}> {}{}", current_time_sec, prefix, message); }

    auto& standard_out = level > LogLevel::LL_WARNING ? uxs::stdbuf::out() : uxs::stdbuf::err();

    uxs::println(standard_out, "\033[0;34m{}> {}{}{}\033[0m", current_time_sec, color, prefix, message);

    if (level == LogLevel::LL_CRITICAL) { throw std::runtime_error(std::string{message}); }
}

void LogManager::registerListener(IListener* listener) {
    std::lock_guard lk(mtx_);
    uxs::binary_insert_unique(listeners_, listener);
}

void LogManager::unregisterListener(IListener* listener) {
    std::lock_guard lk(mtx_);
    uxs::binary_erase_one(listeners_, listener);
}
