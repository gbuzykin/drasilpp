cmake_minimum_required(VERSION 3.21)

project(
  DrasilPP
  DESCRIPTION "DrasilPP Game Infrastructure"
  LANGUAGES CXX)

include(FetchContent)
include(GenerateExportHeader)
include(GNUInstallDirs)

option(OPTION_DEBUG_USE_SANITIZERS "Use Sanitizers for Debug build" ON)

set(INSTALL_WIN32_SYMBOLS_DIR ${CMAKE_INSTALL_BINDIR})

set(CMAKE_CXX_STANDARD
    20
    CACHE STRING "Default C++ standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message("Using C++${CMAKE_CXX_STANDARD} standard")

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

if(UNIX)
  set(CMAKE_SKIP_RPATH OFF)
  set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
  set(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
endif()

# ##############################################################################
# Compilation flags

if(MSVC)
  add_compile_options(/Zc:__cplusplus /utf-8)
else()
  add_compile_options(-Wall -pedantic)
  add_compile_options(-Wno-unused-function)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wno-nullability-extension)
    add_compile_options(-Wno-nullability-completeness)
  endif()
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options($<$<CONFIG:Debug>:-fstandalone-debug>)
  endif()
endif()

if(OPTION_DEBUG_USE_SANITIZERS)
  message("Using sanitizers for Debug build")
  if(MSVC)
    add_compile_options($<$<CONFIG:Debug>:/fsanitize=address>)
  else()
    add_compile_options($<$<CONFIG:Debug>:-fsanitize=address,undefined,leak>)
    add_link_options($<$<CONFIG:Debug>:-fsanitize=address,undefined,leak>)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      add_link_options($<$<CONFIG:Debug>:-shared-libsan>)
    endif()
  endif()
endif()

# ##############################################################################
# Add `uxs` build target

block()
set(UXS_USE_ZLIB OFF)
set(UXS_USE_LIBZIP OFF)
set(UXS_DO_INSTALL_SDK OFF)
FetchContent_Declare(uxs SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/uxs)
FetchContent_MakeAvailable(uxs)
endblock()

# ##############################################################################
# Add `drasilpp` build target

if(WIN32)
  set(DRASILPP_USE_PLATFORM WIN32)
elseif(UNIX)
  set(DRASILPP_USE_PLATFORM XCB)
endif()

set(DRASILPP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
include_directories(${DRASILPP_INCLUDE_DIR})
include_directories(${PROJECT_BINARY_DIR}/generated)

add_subdirectory(src/common)
add_subdirectory(src/rel)
add_subdirectory(src/vulkan)
if(WIN32)
  add_subdirectory(src/d3d12)
endif()

file(GLOB_RECURSE DRASILPP_SRC_FILES src/app/*.h;src/app/*.cpp)

if(DRASILPP_USE_PLATFORM STREQUAL "WIN32")
  file(GLOB_RECURSE DRASILPP_PLATFORM_SRC_FILES
       src/platform/win32/*.h;src/platform/win32/*.cpp)
elseif(DRASILPP_USE_PLATFORM STREQUAL "XCB")
  file(GLOB_RECURSE DRASILPP_PLATFORM_SRC_FILES
       src/platform/xcb/*.h;src/platform/xcb/*.cpp)
endif()

add_executable(drasilpp ${DRASILPP_SRC_FILES} ${DRASILPP_PLATFORM_SRC_FILES})

target_include_directories(drasilpp
                           PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/stb_image)
target_include_directories(drasilpp
                           PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/tiny_obj_loader)
target_link_libraries(drasilpp PRIVATE drasilpp-rel)

if(DRASILPP_USE_PLATFORM STREQUAL "XCB")
  target_link_libraries(drasilpp PRIVATE xcb)
endif()

install(TARGETS drasilpp RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                                 COMPONENT binaries)
install_win32_pdb(drasilpp)

install(
  DIRECTORY ${CMAKE_SOURCE_DIR}/data
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT binaries)

# ##############################################################################
# Auxiliary

add_compile_defs_and_include_dirs_targets(drasilpp ${CMAKE_CURRENT_SOURCE_DIR})
