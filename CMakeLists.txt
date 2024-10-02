cmake_minimum_required(VERSION 3.15)

project(TALCS_REMOTE VERSION 0.0.1)

option(TALCS_REMOTE_BUILD_TESTS "Build Talcs Remote Tests" OFF)

add_subdirectory(src)

if(TALCS_REMOTE_BUILD_TESTS)
    add_subdirectory(test)
endif()