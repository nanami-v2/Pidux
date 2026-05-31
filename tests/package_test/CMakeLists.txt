cmake_minimum_required(VERSION 3.26)
project(package_test LANGUAGES CXX)

find_package(pidux CONFIG REQUIRED)

add_executable(package_test ./src/main.cpp)
set_target_properties(
    package_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/package_test
)
target_link_libraries(package_test PRIVATE pidux::pidux)