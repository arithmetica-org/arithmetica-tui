cmake_minimum_required(VERSION 3.0)

project(arithmetica-tui)

set(sources
    main.cpp
    factor_polynomial.cpp
)

add_executable(arithmetica-tui ${sources})
target_link_libraries(${PROJECT_NAME} PRIVATE arithmetica)
target_link_libraries(${PROJECT_NAME} PRIVATE basic_math_operations)
# Rename target to 'arithmetica'
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME arithmetica)