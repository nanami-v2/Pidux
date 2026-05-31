vcpkg_cmake_configure(
    SOURCE_PATH ${CMAKE_CURRENT_LIST_DIR}/../pidux
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(
    CONFIG_PATH lib/${PORT}/cmake
)

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
file(
    INSTALL ${CMAKE_CURRENT_LIST_DIR}/usage
    DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT}
)
vcpkg_install_copyright(FILE_LIST ${CMAKE_CURRENT_LIST_DIR}/../LICENSE)