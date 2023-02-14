cmake_minimum_required(VERSION 3.15...3.24 FATAL_ERROR)

project(datraw)

add_library(datraw INTERFACE)
target_include_directories(datraw INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/datraw>)
