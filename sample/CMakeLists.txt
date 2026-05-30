cmake_minimum_required(VERSION 3.26)

# https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration
set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

project(sample LANGUAGES CXX)

find_package(boost_container CONFIG REQUIRED)

add_executable(sample ./main.cpp)
set_target_properties(sample PROPERTIES
    CMAKE_CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED ON
    CMAKE_CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
target_include_directories(sample PRIVATE ../${CMAKE_SOURCE_DIR}/pidux/include)
target_link_libraries(sample PRIVATE Boost::container)