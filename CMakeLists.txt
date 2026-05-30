cmake_minimum_required(VERSION 3.26)

# https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration
set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
project(pidux-all VERSION 1.0.0)

add_subdirectory(pidux)

