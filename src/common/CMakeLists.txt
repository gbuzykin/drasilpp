file(GLOB_RECURSE UTILS_INCLUDE_FILES ${DRASILPP_INCLUDE_DIR}/common/*.h)
file(GLOB_RECURSE UTILS_SRC_FILES *.h;*.cpp)

add_library(drasilpp-common SHARED ${UTILS_INCLUDE_FILES} ${UTILS_SRC_FILES})

generate_export_header(
  drasilpp-common EXPORT_FILE_NAME
  ${PROJECT_BINARY_DIR}/generated/common/config.h INCLUDE_GUARD_NAME
  DRASILPP_COMMON_H)

target_link_libraries(drasilpp-common PUBLIC UXS::UXS)

install(
  TARGETS drasilpp-common
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT binaries
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)
install_win32_pdb(drasilpp-common)

add_compile_defs_and_include_dirs_targets(drasilpp-common
                                          ${CMAKE_CURRENT_SOURCE_DIR})
