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
    # Define paths for local dependencies
    set(LOCAL_AM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/arithmetica")
    set(LOCAL_BMO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/basic_math_operations")

    # First fetch the repositories only if they don't already exist
    include(FetchContent)

    if(EXISTS "${LOCAL_BMO_DIR}/CMakeLists.txt")
        message(STATUS "Using existing basic_math_operations in deps/")
        set(bmo_SOURCE_DIR "${LOCAL_BMO_DIR}")
    else()
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
        set(bmo_SOURCE_DIR "${bmo_SOURCE_DIR}")  # Ensure variable is set
    endif()

    if(EXISTS "${LOCAL_AM_DIR}/CMakeLists.txt")
        message(STATUS "Using existing arithmetica in deps/")
        set(am_SOURCE_DIR "${LOCAL_AM_DIR}")
    else()
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
        set(am_SOURCE_DIR "${am_SOURCE_DIR}")  # Ensure variable is set
    endif()

    # Disable unnecessary features
    set(BUILD_BMO OFF)
    set(ENABLE_TESTS OFF)
    set(BENCHMARK OFF)

    # Include library directories
    include_directories(${am_SOURCE_DIR}/src/library/)
    include_directories(${bmo_SOURCE_DIR}/src/library/)
endif()

add_subdirectory(src)

if (ARITHMETICA_TUI_TESTS)
    add_subdirectory(tests)
endif()