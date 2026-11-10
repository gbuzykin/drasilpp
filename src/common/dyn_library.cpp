#include "common/dyn_library.h"  // IWYU pragma: keep

#include "common/log_manager.h"

#include <uxs/string_utf.h>

#include <string>

#if defined(WIN32)
#    include <windows.h>

#    include <memory>
#    define DYN_LIBRARY_PREFIX ""
#    define DYN_LIBRARY_SUFFIX ".dll"
#elif defined(__linux__)
#    include <dlfcn.h>
#    define DYN_LIBRARY_PREFIX "lib"
#    define DYN_LIBRARY_SUFFIX ".so"
#endif

using namespace drasilpp;

void* drasilpp::loadDynLibrary(const std::filesystem::path& library_dir, std::string_view library_name) {
    const auto library_full_name = library_name.starts_with(DYN_LIBRARY_PREFIX) ?
                                       std::string{library_name} + DYN_LIBRARY_SUFFIX :
                                       DYN_LIBRARY_PREFIX + std::string{library_name} + DYN_LIBRARY_SUFFIX;

    std::filesystem::path library_path(uxs::utf_native_path_adapter{}(library_full_name));
    if (!library_dir.empty()) {
        library_path = library_dir / library_path;
#if defined(__linux__)
        if (!library_dir.is_absolute()) { library_path = "../lib" / library_path; }
#endif
        if (!std::filesystem::exists(library_path)) {
            error().printLn("couldn't find dynamic library '{}'", library_path.filename());
            return nullptr;
        }
        library_path = std::filesystem::canonical(library_path);
    }

    std::string error_message;

#if defined(WIN32)
    void* library = ::LoadLibraryW(library_path.c_str());
    if (!library) {
        DWORD error_code = ::GetLastError();
        WCHAR* ptr = nullptr;
        ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code,
                         MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (LPWSTR)&ptr, 0, nullptr);
        if (std::unique_ptr<WCHAR, HLOCAL (*)(HLOCAL)> buffer(ptr, ::LocalFree); buffer.get()) {
            error_message = uxs::utf_string_adapter<char>{}(buffer.get());
        }
    }
#elif defined(__linux__)
    void* library = ::dlopen(library_path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (!library) {
        if (char* psz_error = ::dlerror()) { error_message = psz_error; }
    }
#endif

    if (!library) { error().printLn("couldn't load dynamic library '{}': {}", library_path.filename(), error_message); }

    return library;
}

void drasilpp::freeDynLibrary(void* library) {
    if (!library) { return; }
#if defined(WIN32)
    ::FreeLibrary(static_cast<HMODULE>(library));
#elif defined(__linux__)
    ::dlclose(library);
#endif
}

void* drasilpp::getDynLibraryEntry(void* library, const char* entry_name) {
#if defined(WIN32)
    void* entry = ::GetProcAddress(static_cast<HMODULE>(library), entry_name);
#elif defined(__linux__)
    void* entry = ::dlsym(library, entry_name);
#endif

    if (!entry) { error().printLn("couldn't find entry point '{}'", entry_name); }

    return entry;
}
