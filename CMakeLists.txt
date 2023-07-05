cmake_minimum_required(VERSION 3.25)
project(NetImguiSokol)

set(CMAKE_CXX_STANDARD 17)

# sokol
add_library(sokol INTERFACE)
target_include_directories(sokol INTERFACE 3rd_party/sokol)

# imgui
file(GLOB IMGUI_SRC 3rd_party/imgui/*.cpp)
add_library(imgui ${IMGUI_SRC})
target_include_directories(imgui PUBLIC 3rd_party/imgui)

add_subdirectory(src)
