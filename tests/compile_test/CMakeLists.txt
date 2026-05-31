cmake_minimum_required(VERSION 3.26)
project(compile_test LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    lib_pidux
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../pidux
)
FetchContent_MakeAvailable(lib_pidux)

add_executable(compile_test ./src/main.cpp)
target_link_libraries(compile_test PRIVATE pidux)