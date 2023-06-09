cmake_minimum_required(VERSION 3.0)

project(arithmetica-tui)

option(BUILD_RELEASE "Build for a GitHub release" ON)
option(USE_SUBMODULE_AM_BMO "Use arithmetica and basic_math_operations as submodules" ON)

if(WIN32 OR USE_SUBMODULE_AM_BMO)
    set(BUILD_BMO OFF)
    set(ENABLE_TESTS OFF)
    include_directories(${CMAKE_SOURCE_DIR}/submodules/arithmetica/src/library/)
    include_directories(${CMAKE_SOURCE_DIR}/submodules/basic_math_operations/src/library/)
endif()

add_subdirectory(src)