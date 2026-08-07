FetchContent_Declare(vma SOURCE_DIR
                         ${CMAKE_SOURCE_DIR}/3rdparty/VulkanMemoryAllocator)
FetchContent_MakeAvailable(vma)

file(GLOB_RECURSE REL_VULKAN_SRC_FILES *.h;*.cpp)

add_library(drasilpp-rel-vulkan SHARED ${REL_VULKAN_SRC_FILES})

target_include_directories(drasilpp-rel-vulkan PRIVATE $ENV{VULKAN_SDK}/include)
target_link_libraries(drasilpp-rel-vulkan
                      PRIVATE drasilpp-rel GPUOpen::VulkanMemoryAllocator)

if(WIN32)
  target_compile_definitions(drasilpp-rel-vulkan
                             PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif(UNIX)
  target_compile_definitions(drasilpp-rel-vulkan
                             PRIVATE VK_USE_PLATFORM_XCB_KHR)
endif()

install(
  TARGETS drasilpp-rel-vulkan
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT binaries
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)
install_win32_pdb(drasilpp-rel-vulkan)

add_compile_defs_and_include_dirs_targets(drasilpp-rel-vulkan
                                          ${CMAKE_CURRENT_SOURCE_DIR})
