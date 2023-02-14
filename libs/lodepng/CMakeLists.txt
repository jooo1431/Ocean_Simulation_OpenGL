cmake_minimum_required(VERSION 3.15...3.24 FATAL_ERROR)

project(lodepng)

set(LODEPNG_HEADERS
  lodepng.h)
set(LODEPNG_SOURCES
  lodepng.cpp)

add_library(lodepng STATIC ${LODEPNG_SOURCES} ${LODEPNG_HEADERS})
target_include_directories(lodepng PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
set_target_properties(lodepng PROPERTIES
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
