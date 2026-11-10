#include "common/env_manager.h"  // IWYU pragma: keep

#include <chrono>

#ifdef __GNUC__
#    include <ctime>
#endif

#if defined(WIN32)
#    include <userenv.h>
#    include <windows.h>
#elif defined(__linux__)
#    include <linux/limits.h>
#    include <sys/types.h>

#    include <pwd.h>
#    include <unistd.h>

#    include <cstdlib>
#endif

using namespace drasilpp;

EnvManager::EnvManager() {}

EnvManager& EnvManager::instance() {
    static EnvManager env_manager;
    return env_manager;
}

void EnvManager::init(std::string_view home_dir_name) {
    initHostName();
    initCurrentTimeZoneOffset();
    initHomePath(home_dir_name);
    initAppPath();
}

void EnvManager::initHostName() {
#if defined(WIN32) || defined(__linux__)
#    if defined(WIN32)
    WORD version = MAKEWORD(2, 2);
    WSADATA wsa_data;
    ::WSAStartup(version, &wsa_data);
#    endif
    char host_name_buf[256];
    const int result = ::gethostname(host_name_buf, sizeof(host_name_buf));
    host_name_ = result >= 0 ? host_name_buf : "NOHOST";
#endif
}

void EnvManager::initCurrentTimeZoneOffset() {
    using namespace std::chrono;
    using namespace std::chrono_literals;
#if defined(__GNUC__)
    std::tm tm{};
    tm.tm_year = 100;
    tm.tm_mday = 1;
    std::time_t t = std::mktime(&tm);
    std::tm gmtm = *std::gmtime(&t);

    const auto tz_offset = sys_days{year_month_day{2000y, January, 1d}} -
                           sys_days{year_month_day{year{1900 + gmtm.tm_year}, month{1U + unsigned(gmtm.tm_mon)},
                                                   day{unsigned(gmtm.tm_mday)}}} -
                           hours(gmtm.tm_hour) - minutes(gmtm.tm_min) - seconds(gmtm.tm_sec);
#else
    const auto tz_offset = current_zone()->get_info(system_clock::now()).offset;
#endif
    tz_offset_ = int(tz_offset.count());
}

void EnvManager::initHomePath(std::string_view home_dir_name) {
#if defined(WIN32)
    WCHAR path[MAX_PATH];
    const DWORD len = ::GetEnvironmentVariableW(L"APPDATA", path, MAX_PATH);
    if (len > 0 && len <= MAX_PATH) {
        home_path_ = std::filesystem::path{std::wstring_view(path, len)};
    } else {
        DWORD len = MAX_PATH;
        if (::GetUserProfileDirectoryW(::GetCurrentProcessToken(), path, &len) && len > 0 && len <= MAX_PATH) {
            home_path_ = std::filesystem::path{std::wstring_view(path, len - 1)};
            home_path_ /= "Application Data";
        }
    }
    home_path_ /= home_dir_name;
#elif defined(__linux__)
    const uid_t uid = ::getuid();
    passwd* pw = ::getpwuid(uid);
    home_path_ = std::filesystem::path{pw && pw->pw_dir ? pw->pw_dir : std::getenv("HOME")};
    home_path_ /= ".";
    home_path_ += home_dir_name;
#endif
    if (!std::filesystem::exists(home_path_)) { std::filesystem::create_directories(home_path_); }
    home_path_ = std::filesystem::canonical(home_path_);
}

void EnvManager::initAppPath() {
#if defined(WIN32)
    WCHAR path[MAX_PATH];
    const DWORD len = ::GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (len > 0 && len <= MAX_PATH) { app_path_ = std::filesystem::path{std::wstring_view(path, len)}; }
#elif defined(__linux__)
    char path[PATH_MAX];
    const ssize_t count = ::readlink("/proc/self/exe", path, sizeof(path));
    if (count >= 0 && count <= PATH_MAX) { app_path_ = std::filesystem::path{std::string_view(path, count)}; }
#endif
    app_path_ = std::filesystem::canonical(app_path_.parent_path());
}
