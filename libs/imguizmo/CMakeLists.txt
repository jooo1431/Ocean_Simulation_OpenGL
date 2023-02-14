cmake_minimum_required(VERSION 3.15...3.24 FATAL_ERROR)

project(imgui)

set(IMGUIZMO_HEADERS
  imGuIZMO.quat/imGuIZMOquat.h
  imGuIZMO.quat/imGuIZMOquat.h
  imGuIZMO.quat/vgConfig.h
  imGuIZMO.quat/vGizmo.h
  imGuIZMO.quat/vGizmoMath.h
  imGuIZMO.quat/vgMath.h)
set(IMGUIZMO_SOURCES
  imGuIZMO.quat/imGuIZMOquat.cpp)

add_library(imguizmo STATIC ${IMGUIZMO_SOURCES} ${IMGUIZMO_HEADERS})
target_include_directories(imguizmo PUBLIC imGuIZMO.quat)
target_compile_definitions(imguizmo
  PRIVATE
    VGIZMO_USES_GLM
    IMGUIZMO_IMGUI_FOLDER=./)
target_compile_features(imguizmo PUBLIC cxx_std_17)
set_target_properties(imguizmo PROPERTIES
  CXX_EXTENSIONS OFF
  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
