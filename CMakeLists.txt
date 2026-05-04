cmake_minimum_required(VERSION 3.26)

# https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration
set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
project(Pipeline VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${CMAKE_SOURCE_DIR}/include)
if (MSVC)
    add_compile_options("/utf-8")
endif()

find_package(boost_container CONFIG REQUIRED)
link_libraries(Boost::container)


add_executable(sample sample/main.cpp)