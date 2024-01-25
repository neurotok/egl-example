cmake_minimum_required(VERSION 3.10)
project(EGLX11OpenGLExample)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the X11 package
find_package(X11 REQUIRED)
if(NOT X11_FOUND)
    message(FATAL_ERROR "X11 not found")
endif()

# Find the EGL library
find_library(EGL_LIBRARY NAMES EGL egl)
if(NOT EGL_LIBRARY)
    message(FATAL_ERROR "EGL library not found")
endif()

# Find the OpenGL ES v2 library
find_library(GLESv2_LIBRARY NAMES GLESv2)
if(NOT GLESv2_LIBRARY)
    message(FATAL_ERROR "OpenGL ES v2 library not found")
endif()

# Add executable target with source files listed
add_executable(EGLX11OpenGLExample main.cpp)

# Include directories for X11
include_directories(${X11_INCLUDE_DIR})

# Link the executable with the X11, EGL, and GLESv2 libraries
target_link_libraries(EGLX11OpenGLExample ${X11_LIBRARIES} ${EGL_LIBRARY} ${GLESv2_LIBRARY})
