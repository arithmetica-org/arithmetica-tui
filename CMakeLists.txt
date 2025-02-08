cmake_minimum_required(VERSION 3.5)

project(arithmetica-tui)

enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_RELEASE "Build for a GitHub release" ON)
option(BUILD_MAIN_EXECUTABLE "Build the main executable" ON)
option(USE_SUBMODULE_AM_BMO "Fetch arithmetica and basic_math_operations using FetchContent" ON)
option(ARITHMETICA_TUI_TESTS "Build tests" OFF)

if(WIN32 OR USE_SUBMODULE_AM_BMO)
    # First fetch the repositories
    include(FetchContent)

    message(STATUS "Fetching basic_math_operations...")
    FetchContent_Declare(bmo
        GIT_REPOSITORY https://github.com/avighnac/basic_math_operations
        GIT_TAG main
    )
    FetchContent_GetProperties(bmo)
    if(NOT bmo_POPULATED)
        message(STATUS "Populating basic_math_operations...")
        FetchContent_Populate(bmo)
    endif()

    message(STATUS "Fetching arithmetica...")
    FetchContent_Declare(am
        GIT_REPOSITORY https://github.com/arithmetica-org/arithmetica
        GIT_TAG main
    )
    FetchContent_GetProperties(am)
    if(NOT am_POPULATED)
        message(STATUS "Populating arithmetica...")
        FetchContent_Populate(am)
    endif()

    set(BUILD_BMO OFF)
    set(ENABLE_TESTS OFF)
    set(BENCHMARK OFF)
    include_directories(${am_SOURCE_DIR}/src/library/)
    include_directories(${bmo_SOURCE_DIR}/src/library/)
endif()

add_subdirectory(src)

if (ARITHMETICA_TUI_TESTS)
    add_subdirectory(tests)
endif()