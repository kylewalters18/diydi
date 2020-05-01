cmake_minimum_required(VERSION 3.1)

project(diydi_benchmarks)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(BENCHMARK_ENABLE_TESTING OFF CACHE INTERNAL "" FORCE)
add_subdirectory(../external/benchmark build_benchmarks)

include_directories(../include)

add_executable(benchmarks diydi.cpp)
target_link_libraries(benchmarks benchmark::benchmark)
