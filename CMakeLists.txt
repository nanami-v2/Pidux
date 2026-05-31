cmake_minimum_required(VERSION 3.26)

# https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration
set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

project(pidux-repo)
option(BUILD_SAMPLE "Build sample" OFF)

add_subdirectory(pidux)
if(BUILD_SAMPLE)
    add_subdirectory(sample)
endif()

