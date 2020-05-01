cmake_minimum_required(VERSION 3.1)

project(diydi)

option(DIYDI_BuildTests "Build the unit tests." ON)

add_library(diydi INTERFACE)
target_include_directories(diydi INTERFACE include)

if(DIYDI_BuildTests)
    enable_testing()
    add_subdirectory(test)
endif()
