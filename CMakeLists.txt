cmake_minimum_required(VERSION 3.1)

project(diydi)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

enable_testing()
add_subdirectory(googletest)

include_directories("include")

add_executable(unit_tests test/diydi.cpp test/graph.cpp)

target_link_libraries(unit_tests gtest gmock gtest_main)
