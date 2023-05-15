cmake_minimum_required(VERSION 3.0)

project(arithmetica-tui)

set(sources
    main.cpp
    factor_polynomial.cpp
    eval_with_steps.cpp
)

add_executable(arithmetica-tui ${sources})
if (WIN32)
    # We'll need to build the library and then link it.
    add_subdirectory(../submodules/arithmetica ${CMAKE_CURRENT_BINARY_DIR}/arithmetica)
    add_subdirectory(../submodules/basic_math_operations ${CMAKE_CURRENT_BINARY_DIR}/basic_math_operations)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE arithmetica)
target_link_libraries(${PROJECT_NAME} PRIVATE basic_math_operations)

if (BUILD_RELEASE)
    # Add '-static -static-libstdc++ -static-libgcc' compiler flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libstdc++ -static-libgcc")
endif()

# Rename target to 'arithmetica'
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME arithmetica)