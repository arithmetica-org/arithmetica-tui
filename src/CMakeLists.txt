cmake_minimum_required(VERSION 3.0)

project(arithmetica-tui)

set(sources
    arithmetica_tui.cpp
    call_arithmetica_tui.cpp
    factor_polynomial.cpp
    eval_with_steps.cpp
    divide_with_steps.cpp
    algnum.cpp
)

if (WIN32 OR USE_SUBMODULE_AM_BMO)
    # We'll need to build the library and then link it.
    add_subdirectory(${am_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/libs/arithmetica)
    add_subdirectory(${bmo_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/libs/basic_math_operations)
endif()

add_library(arithmetica-tui-lib STATIC ${sources})
target_link_libraries(arithmetica-tui-lib PRIVATE arithmetica)
target_link_libraries(arithmetica-tui-lib PRIVATE basic_math_operations)

if (BUILD_MAIN_EXECUTABLE)
    # If in debug mode, modify flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -O0")

    add_executable(arithmetica-tui main.cpp)
    target_link_libraries(arithmetica-tui PRIVATE arithmetica-tui-lib)
    if (BUILD_RELEASE)
        # Add '-static -static-libstdc++ -static-libgcc' compiler flags
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libstdc++ -static-libgcc")
    endif()

    # Rename target to 'arithmetica'
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME arithmetica)
endif()