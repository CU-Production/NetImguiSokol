#set(CMAKE_EXECUTABLE_SUFFIX ".html")
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
        Source/html5/NetImguiServer_App_emsc.cpp
        Source/html5/NetImguiServer_HAL_emsc.cpp
        Source/html5/NetImguiServer_HAL_Sgfx.cpp
        )
#target_compile_options(NetImguiServerApp PRIVATE -pthread -g)
target_compile_options(NetImguiServerApp PRIVATE -pthread -Oz --closure 1)
target_link_libraries(NetImguiServerApp PRIVATE NetImguiClientLib imgui sokol
        -sMAX_WEBGL_VERSION=2
        -sUSE_PTHREADS=1
        -sPTHREAD_POOL_SIZE=4
        "--preload-file Background.png"
        -lwebsocket.js
        -sALLOW_MEMORY_GROWTH
        -sOFFSCREEN_FRAMEBUFFER=1
        -sPROXY_TO_PTHREAD=1
        -sPROXY_POSIX_SOCKETS=1
        -sOFFSCREENCANVAS_SUPPORT
        -flto
        -sWASM_BIGINT
        "--closure 1"
        )
target_include_directories(NetImguiServerApp PRIVATE
        Source
        ThirdParty/DearImgui)
add_custom_command(
        TARGET NetImguiServerApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ARGS ${CMAKE_CURRENT_SOURCE_DIR}/Background.png ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ARGS ${CMAKE_CURRENT_SOURCE_DIR}/coi-serviceworker.min.js ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ARGS ${CMAKE_CURRENT_SOURCE_DIR}/NetImguiServerApp.html ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ARGS ${CMAKE_CURRENT_SOURCE_DIR}/servewasm.py ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ARGS ${CMAKE_CURRENT_SOURCE_DIR}/websocket_to_posix_proxy.exe ${CMAKE_CURRENT_BINARY_DIR}
)
