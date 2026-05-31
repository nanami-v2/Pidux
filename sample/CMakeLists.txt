cmake_minimum_required(VERSION 3.26)
project(sample LANGUAGES CXX)

find_package(Boost CONFIG REQUIRED COMPONENTS container)

add_executable(sample ./main.cpp)
set_target_properties(
    sample PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
target_compile_features(sample PRIVATE cxx_std_17)
target_link_libraries(sample PRIVATE Boost::container)
target_include_directories(sample PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../pidux/include)