cmake_minimum_required(VERSION 3.26)

project(pidux-repo)
option(BUILD_SAMPLE "Build sample" OFF)

add_subdirectory(pidux)
if(BUILD_SAMPLE)
    add_subdirectory(sample)
endif()

