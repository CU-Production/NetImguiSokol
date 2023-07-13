#set(CMAKE_EXECUTABLE_SUFFIX ".html")
add_executable(NetImguiSokol main.cpp)
#target_link_libraries(NetImguiSokol PRIVATE sokol imgui X11 EGL GLESv2 dl pthread m -sMAX_WEBGL_VERSION=2)
target_link_libraries(NetImguiSokol PRIVATE sokol imgui -sMAX_WEBGL_VERSION=2)
