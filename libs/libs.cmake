# Dependencies

# Setup FetchContent
include(FetchContent)
mark_as_advanced(FORCE
  FETCHCONTENT_BASE_DIR
  FETCHCONTENT_FULLY_DISCONNECTED
  FETCHCONTENT_QUIET
  FETCHCONTENT_UPDATES_DISCONNECTED)

# GLFW
FetchContent_Declare(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.3.8
  GIT_SHALLOW TRUE)
FetchContent_GetProperties(glfw)
if (NOT glfw_POPULATED)
  message(STATUS "Fetch glfw ...")
  FetchContent_Populate(glfw)
  option(GLFW_BUILD_DOCS "" OFF)
  option(GLFW_BUILD_EXAMPLES "" OFF)
  option(GLFW_BUILD_TESTS "" OFF)
  option(GLFW_INSTALL "" OFF)
  if (WIN32)
    option(GLFW_USE_HYBRID_HPG "" ON)
    option(USE_MSVC_RUNTIME_LIBRARY_DLL "" OFF)
  elseif (UNIX AND NOT APPLE)
    # Detect X11 or Wayland
    if ("$ENV{XDG_SESSION_TYPE}" STREQUAL "wayland")
      option(GLFW_USE_WAYLAND "" ON)
    endif ()
  endif ()
  add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR} EXCLUDE_FROM_ALL)
  set_target_properties(glfw PROPERTIES FOLDER libs)
  mark_as_advanced(FORCE
    FETCHCONTENT_SOURCE_DIR_GLFW
    FETCHCONTENT_UPDATES_DISCONNECTED_GLFW
    BUILD_SHARED_LIBS
    GLFW_BUILD_DOCS
    GLFW_BUILD_EXAMPLES
    GLFW_BUILD_TESTS
    GLFW_INSTALL
    GLFW_USE_HYBRID_HPG
    GLFW_USE_OSMESA
    GLFW_USE_WAYLAND
    GLFW_VULKAN_STATIC
    USE_MSVC_RUNTIME_LIBRARY_DLL
    X11_xcb_icccm_INCLUDE_PATH
    X11_xcb_icccm_LIB)
endif ()

# glad2
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/glad/ EXCLUDE_FROM_ALL)
set_target_properties(glad PROPERTIES FOLDER libs)

# glm
FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 0.9.9.8
  GIT_SHALLOW TRUE)
FetchContent_GetProperties(glm)
if (NOT glm_POPULATED)
  message(STATUS "Fetch glm ...")
  FetchContent_Populate(glm)
  add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR} EXCLUDE_FROM_ALL)
  mark_as_advanced(FORCE
    FETCHCONTENT_SOURCE_DIR_GLM
    FETCHCONTENT_UPDATES_DISCONNECTED_GLM)
endif ()

# glowl
FetchContent_Declare(glowl
  GIT_REPOSITORY https://github.com/invor/glowl.git
  GIT_TAG e80ae434618d7a3b0056f2765dcca9d6d64c1db7)
FetchContent_GetProperties(glowl)
if (NOT glowl_POPULATED)
  message(STATUS "Fetch glowl ...")
  FetchContent_Populate(glowl)
  set(GLOWL_OPENGL_INCLUDE "GLAD2" CACHE STRING "" FORCE)
  option(GLOWL_USE_ARB_BINDLESS_TEXTURE "" OFF)
  add_subdirectory(${glowl_SOURCE_DIR} ${glowl_BINARY_DIR} EXCLUDE_FROM_ALL)
  # Mark include dirs as 'system' to disable warnings.
  get_target_property(include_dirs glowl INTERFACE_INCLUDE_DIRECTORIES)
  set_target_properties(glowl PROPERTIES
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${include_dirs}")
  mark_as_advanced(FORCE
    FETCHCONTENT_SOURCE_DIR_GLOWL
    FETCHCONTENT_UPDATES_DISCONNECTED_GLOWL
    GLOWL_OPENGL_INCLUDE
    GLOWL_USE_ARB_BINDLESS_TEXTURE
    GLOWL_USE_GLM
    GLOWL_USE_NV_MESH_SHADER)
endif ()

# imgui
FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.88
  GIT_SHALLOW TRUE)
FetchContent_GetProperties(imgui)
if (NOT imgui_POPULATED)
  message(STATUS "Fetch imgui ...")
  FetchContent_Populate(imgui)
  file(COPY ${CMAKE_SOURCE_DIR}/libs/imgui/CMakeLists.txt DESTINATION ${imgui_SOURCE_DIR})
  add_subdirectory(${imgui_SOURCE_DIR} ${imgui_BINARY_DIR} EXCLUDE_FROM_ALL)
  target_link_libraries(imgui PRIVATE glfw)
  set_target_properties(imgui PROPERTIES FOLDER libs)
  mark_as_advanced(FORCE
    FETCHCONTENT_SOURCE_DIR_IMGUI
    FETCHCONTENT_UPDATES_DISCONNECTED_IMGUI)
endif ()

# imGuIZMO.quat
FetchContent_Declare(imguizmo
  GIT_REPOSITORY https://github.com/BrutPitt/imGuIZMO.quat.git
  GIT_TAG v3.0
  GIT_SHALLOW TRUE)
FetchContent_GetProperties(imguizmo)
if (NOT imguizmo_POPULATED)
  message(STATUS "Fetch imguizmo ...")
  FetchContent_Populate(imguizmo)
  file(COPY ${CMAKE_SOURCE_DIR}/libs/imguizmo/CMakeLists.txt DESTINATION ${imguizmo_SOURCE_DIR})
  add_subdirectory(${imguizmo_SOURCE_DIR} ${imguizmo_BINARY_DIR} EXCLUDE_FROM_ALL)
  target_link_libraries(imguizmo PRIVATE imgui glm)
  get_target_property(include_dirs imguizmo INTERFACE_INCLUDE_DIRECTORIES)
  set_target_properties(imguizmo PROPERTIES
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${include_dirs}"
    FOLDER libs)
  mark_as_advanced(FORCE
    FETCHCONTENT_SOURCE_DIR_IMGUIZMO
    FETCHCONTENT_UPDATES_DISCONNECTED_IMGUIZMO)
endif ()

# LodePNG
FetchContent_Declare(lodepng
  GIT_REPOSITORY https://github.com/lvandeve/lodepng.git
  GIT_TAG 18964554bc769255401942e0e6dfd09f2fab2093)
FetchContent_GetProperties(lodepng)
if (NOT lodepng_POPULATED)
  message(STATUS "Fetch lodepng ...")
  FetchContent_Populate(lodepng)
  file(COPY ${CMAKE_SOURCE_DIR}/libs/lodepng/CMakeLists.txt DESTINATION ${lodepng_SOURCE_DIR})
  add_subdirectory(${lodepng_SOURCE_DIR} ${lodepng_BINARY_DIR} EXCLUDE_FROM_ALL)
  set_target_properties(lodepng PROPERTIES FOLDER libs)
  mark_as_advanced(FORCE
    FETCHCONTENT_SOURCE_DIR_LODEPNG
    FETCHCONTENT_UPDATES_DISCONNECTED_LODEPNG)
endif ()

# datraw
FetchContent_Declare(datraw
  GIT_REPOSITORY https://github.com/UniStuttgart-VISUS/datraw.git
  GIT_TAG v1.0.6
  GIT_SHALLOW TRUE)
FetchContent_GetProperties(datraw)
if (NOT datraw_POPULATED)
  message(STATUS "Fetch datraw ...")
  FetchContent_Populate(datraw)
  file(COPY ${CMAKE_SOURCE_DIR}/libs/datraw/CMakeLists.txt DESTINATION ${datraw_SOURCE_DIR})
  add_subdirectory(${datraw_SOURCE_DIR} ${datraw_BINARY_DIR} EXCLUDE_FROM_ALL)
  get_target_property(include_dirs datraw INTERFACE_INCLUDE_DIRECTORIES)
  set_target_properties(datraw PROPERTIES
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${include_dirs}")
  mark_as_advanced(FORCE
    FETCHCONTENT_SOURCE_DIR_DATRAW
    FETCHCONTENT_UPDATES_DISCONNECTED_DATRAW)
endif ()

# boost stacktrace
if (OGL4CORE2_ENABLE_STACKTRACE)
  FetchContent_Declare(stacktrace
    URL "https://github.com/UniStuttgart-VISUS/boost-stacktrace-zip/releases/download/boost-1.80.0/boost-stacktrace.zip"
    URL_HASH SHA256=394c963eaef339d32d08162a4253a96bbc5197d768133877e29fa6ad5bdd7570)
  FetchContent_GetProperties(stacktrace)
  if (NOT stacktrace_POPULATED)
    message(STATUS "Fetch stacktrace ...")
    FetchContent_Populate(stacktrace)
    add_subdirectory(${stacktrace_SOURCE_DIR} ${stacktrace_BINARY_DIR} EXCLUDE_FROM_ALL)
    if (WIN32)
      set_target_properties(boost_stacktrace_windbg PROPERTIES FOLDER libs)
    endif ()
    mark_as_advanced(FORCE
      FETCHCONTENT_SOURCE_DIR_STACKTRACE
      FETCHCONTENT_UPDATES_DISCONNECTED_STACKTRACE
      BOOST_REGEX_STANDALONE
      BOOST_STACKTRACE_ENABLE_ADDR2LINE
      BOOST_STACKTRACE_ENABLE_BACKTRACE
      BOOST_STACKTRACE_ENABLE_BASIC
      BOOST_STACKTRACE_ENABLE_NOOP
      BOOST_STACKTRACE_ENABLE_WINDBG
      BOOST_STACKTRACE_ENABLE_WINDBG_CACHED
      BOOST_THREAD_THREADAPI
      ICU_INCLUDE_DIR)
  endif ()
endif ()
