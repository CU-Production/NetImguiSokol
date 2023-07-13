add_executable(NetImguiClientApp main.cpp)
target_link_libraries(NetImguiClientApp PRIVATE NetImguiClientLib imgui sokol
    X11 EGL GLESv2 dl pthread m)
