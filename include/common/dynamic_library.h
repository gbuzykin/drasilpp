#pragma once

#include "common/config.h"

#include <filesystem>
#include <string_view>

namespace drasilpp {

DRASILPP_COMMON_EXPORT void* loadDynamicLibrary(const std::filesystem::path& library_dir, std::string_view library_name);
DRASILPP_COMMON_EXPORT void freeDynamicLibrary(void* library);
DRASILPP_COMMON_EXPORT void* getDynamicLibraryEntry(void* library, const char* entry_name);

}  // namespace drasilpp
