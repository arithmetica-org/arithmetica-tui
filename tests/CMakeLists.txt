cmake_minimum_required(VERSION 3.16)

project(arithmetica_tui_tests)

enable_language(CXX)

# Download and build Google Test if necessary
if(NOT TARGET gtest)
  include(FetchContent)

  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.13.0)

  FetchContent_GetProperties(googletest)

  if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
  endif()
endif()

# Add test sources
set(test_sources
    factor_tests.cpp
    eval_tests.cpp)

add_executable(${PROJECT_NAME} ${test_sources})
target_link_libraries(${PROJECT_NAME} PRIVATE arithmetica-tui-lib gtest_main)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})