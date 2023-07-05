add_executable(NetImguiServerApp
        Source/NetImguiServer_App.cpp
        Source/NetImguiServer_App.h
        Source/NetImguiServer_Config.cpp
        Source/NetImguiServer_Config.h
        Source/NetImguiServer_Network.cpp
        Source/NetImguiServer_Network.h
        Source/NetImguiServer_RemoteClient.cpp
        Source/NetImguiServer_RemoteClient.h
        Source/NetImguiServer_UI.cpp
        Source/NetImguiServer_UI.h
        Source/Win32DX11/NetImguiServer_App_win32dx11.cpp
        Source/Win32DX11/NetImguiServer_HAL_dx11.cpp
        Source/Win32DX11/NetImguiServer_HAL_win32.cpp
        Source/Sokol/NetImguiServer_App_Sokol.cpp
        Source/Sokol/NetImguiServer_HAL_Sapp.cpp
        Source/Sokol/NetImguiServer_HAL_Sgfx.cpp
        )
target_link_libraries(NetImguiServerApp PRIVATE NetImguiClientLib imgui sokol)
target_include_directories(NetImguiServerApp PRIVATE
        Source
        ThirdParty/DearImgui)
add_custom_command(
        TARGET NetImguiServerApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ARGS ${CMAKE_CURRENT_SOURCE_DIR}/Background.png ${CMAKE_CURRENT_BINARY_DIR}
)
