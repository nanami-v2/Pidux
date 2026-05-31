cmake_minimum_required(VERSION 3.26)
project(pidux VERSION 1.1.0 LANGUAGES CXX)

include(GnuInstallDirs)
include(CMakePackageConfigHelpers)

find_package(Boost CONFIG REQUIRED COMPONENTS container)

add_library(pidux INTERFACE)
target_compile_features(pidux INTERFACE cxx_std_17)
target_link_libraries(pidux INTERFACE Boost::container)
target_include_directories(
    pidux INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# install: header files
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
# install: target export file
install(TARGETS pidux EXPORT piduxTargets)
install(
    EXPORT piduxTargets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pidux/cmake
    NAMESPACE pidux::
    FILE "pidux-targets.cmake"
)
# install: config file
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/pidux-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/pidux-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/pidux/cmake
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/pidux-config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pidux/cmake
)
# install: version file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/pidux-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/pidux-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pidux/cmake
)