#pragma once

#include "common/config.h"

#include <filesystem>
#include <string>
#include <string_view>

namespace drasilpp {

class DRASILPP_COMMON_EXPORT EnvManager {
 public:
    static EnvManager& instance();

    void init(std::string_view home_dir_name);

    // Local host name
    const std::string& getLocalHostName() const { return host_name_; }

    // Current time zone offset
    int getCurrentTimeZoneOffset() const { return tz_offset_; }

    // Standard paths
    const std::filesystem::path& getHomePath() const { return home_path_; }
    const std::filesystem::path& getAppPath() const { return app_path_; }

 private:
    EnvManager();

    void initHostName();
    void initCurrentTimeZoneOffset();
    void initHomePath(std::string_view home_dir_name);
    void initAppPath();

    std::string host_name_;
    int tz_offset_;
    std::filesystem::path home_path_;
    std::filesystem::path app_path_;
};

inline EnvManager& getEnvManager() { return EnvManager::instance(); }

}  // namespace drasilpp
