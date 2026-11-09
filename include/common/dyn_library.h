#pragma once

#include "common/config.h"

#include <filesystem>
#include <string_view>

namespace drasilpp {

DRASILPP_COMMON_EXPORT void* loadDynLibrary(const std::filesystem::path& library_dir, std::string_view library_name);
DRASILPP_COMMON_EXPORT void freeDynLibrary(void* library);
DRASILPP_COMMON_EXPORT void* getDynLibraryEntry(void* library, const char* entry_name);

}  // namespace drasilpp
