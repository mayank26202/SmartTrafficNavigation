cmake_minimum_required(VERSION 3.10)
project(SmartTrafficNavigation)

set(CMAKE_CXX_STANDARD 17)

# Include header files
include_directories(include)

# Add source files
add_executable(SmartTrafficNavigation
    src/main.cpp
)

# ✅ Link with OpenGL + FreeGLUT
find_package(OpenGL REQUIRED)

# Manually link FreeGLUT (Windows)
target_link_libraries(SmartTrafficNavigation
    ${OPENGL_LIBRARIES}
    freeglut
    glu32
    opengl32
)
