cmake_minimum_required(VERSION 3.5)

project(arithmetica-tui)

set(sources
    arithmetica_tui.cpp
    call_arithmetica_tui.cpp
    factor_polynomial.cpp
    eval_with_steps.cpp
    divide_with_steps.cpp
    algnum.cpp
    invert_matrix.cpp
    matmul.cpp
)

set(helper_sources
    helpers/center.cpp
    helpers/compare_printed_text.cpp
    helpers/compare_text_without_ansi.cpp
    helpers/get_console_width.cpp
    helpers/get_matching_brace.cpp
    helpers/get_printable_result.cpp
    helpers/getch.cpp
    helpers/is_valid_arithmetic_expression.cpp
    helpers/print_eval_expression.cpp
    helpers/print_expression.cpp
    helpers/print_result.cpp
    helpers/remove_extra_front_back_brackets.cpp
    helpers/replace_all.cpp
    helpers/round_decimal.cpp
    helpers/tokenize.cpp)

if (WIN32 OR USE_SUBMODULE_AM_BMO)
    # We'll need to build the library and then link it.
    add_subdirectory(${am_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/libs/arithmetica)
    add_subdirectory(${bmo_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/libs/basic_math_operations)
endif()

add_library(arithmetica-tui-lib STATIC ${sources} ${helper_sources})
target_link_libraries(arithmetica-tui-lib PRIVATE arithmetica)
target_link_libraries(arithmetica-tui-lib PRIVATE basic_math_operations)

target_include_directories(arithmetica-tui-lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(arithmetica-tui-lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/helpers)
target_include_directories(arithmetica-tui-lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/functions)

if (BUILD_MAIN_EXECUTABLE)
    add_executable(arithmetica-tui main.cpp)
    target_link_libraries(arithmetica-tui PRIVATE arithmetica-tui-lib)
    if (BUILD_RELEASE)
    if (NOT APPLE)
        # Add '-static -static-libstdc++ -static-libgcc' compiler flags
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libstdc++ -static-libgcc")
    endif()
    endif()
    
    # Add an icon if we're on Windows
    if (WIN32)
        # Set the path to the icon file
        set(ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../icon.ico")

        # Generate the resource script
        set(RESOURCE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/icon.rc")
        file(WRITE ${RESOURCE_SCRIPT} "1 ICON \"${ICON_FILE}\"")

        # Compile the resource script using windres
        set(RESOURCE_OBJECT "${CMAKE_CURRENT_BINARY_DIR}/icon.o")
        execute_process(COMMAND windres ${RESOURCE_SCRIPT} -O coff -o ${RESOURCE_OBJECT})

        # Link the resource object with the target executable
        target_link_libraries(arithmetica-tui PRIVATE ${RESOURCE_OBJECT})
    endif()

    # Rename target to 'arithmetica'
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME arithmetica)
endif()