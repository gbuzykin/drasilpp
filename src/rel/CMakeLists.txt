file(GLOB_RECURSE REL_INCLUDE_FILES ${DRASILPP_INCLUDE_DIR}/rel/*.h)
file(GLOB_RECURSE REL_SRC_FILES *.h;*.cpp)

add_library(drasilpp-rel SHARED ${REL_INCLUDE_FILES} ${REL_SRC_FILES})

generate_export_header(
  drasilpp-rel EXPORT_FILE_NAME ${PROJECT_BINARY_DIR}/generated/rel/config.h
  INCLUDE_GUARD_NAME DRASILPP_REL_H)

target_include_directories(drasilpp-rel PRIVATE $ENV{VULKAN_SDK}/include)
target_link_libraries(drasilpp-rel PUBLIC drasilpp-common)

install(
  TARGETS drasilpp-rel
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT binaries
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)
install_win32_pdb(drasilpp-rel)

add_compile_defs_and_include_dirs_targets(drasilpp-rel
                                          ${CMAKE_CURRENT_SOURCE_DIR})
