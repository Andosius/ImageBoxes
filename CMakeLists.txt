cmake_minimum_required(VERSION 3.20)

# Project declaration
project(ImageBoxes)

set(EXE_NAME "Image2Boxes")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -Wformat")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin" CACHE STRING "" FORCE)

set(SOURCE_FILES
        Source/main.cpp
        Source/Application.cpp
        Source/ApplicationLayer.cpp
        Source/ImageUtility.cpp)

set(IMGUI_FILES
	Dependencies/imgui/imgui.cpp
	Dependencies/imgui/imgui_demo.cpp
	Dependencies/imgui/imgui_draw.cpp
	Dependencies/imgui/imgui_tables.cpp
	Dependencies/imgui/imgui_widgets.cpp
	Dependencies/imgui/backends/imgui_impl_glfw.cpp
	Dependencies/imgui/backends/imgui_impl_opengl3.cpp)

# Include GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(Dependencies/glfw)

# Include DearImGui
include_directories(Dependencies/imgui Dependencies/imgui/backends)

# Include stb
include_directories(Dependencies/stb)

# Add Native File Dialog (Extended)
add_subdirectory(Dependencies/nativefiledialog-extended)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Generate executable program
add_executable(${EXE_NAME} ${SOURCE_FILES} ${IMGUI_FILES})

target_link_libraries(${EXE_NAME} PUBLIC glfw OpenGL::GL nfd)

# Set Visual Studio startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ImageBoxes)
