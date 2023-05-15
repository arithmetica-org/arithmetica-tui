cmake_minimum_required(VERSION 3.0)

project(arithmetica-tui)

if(WIN32)
    set(BUILD_BMO OFF)
    set(ENABLE_TESTS OFF)
    include_directories(${CMAKE_SOURCE_DIR}/submodules/arithmetica/src/library/)
    include_directories(${CMAKE_SOURCE_DIR}/submodules/basic_math_operations/src/library/)
endif(WIN32)

option(BUILD_RELEASE "Build for a GitHub release" ON)

add_subdirectory(src)