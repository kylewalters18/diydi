cmake_minimum_required(VERSION 2.8)

project(diydi)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

enable_testing()
add_subdirectory(googletest)

include_directories("include")

add_executable(unit_tests
  test/tests.cpp
)

target_link_libraries(unit_tests gtest gmock gtest_main)
