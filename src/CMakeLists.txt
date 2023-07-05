add_subdirectory(SokolImgui)

add_library(NetImguiClientLib
        ../3rd_party/netImgui/Client/Private/NetImgui_Api.cpp
        ../3rd_party/netImgui/Client/Private/NetImgui_Client.cpp
        ../3rd_party/netImgui/Client/Private/NetImgui_CmdPackets_DrawFrame.cpp
        ../3rd_party/netImgui/Client/Private/NetImgui_NetworkPosix.cpp
        ../3rd_party/netImgui/Client/Private/NetImgui_NetworkUE4.cpp
        ../3rd_party/netImgui/Client/Private/NetImgui_NetworkWin32.cpp)
target_include_directories(NetImguiClientLib PUBLIC ../3rd_party/netImgui/Client)
target_link_libraries(NetImguiClientLib PRIVATE imgui)

add_subdirectory(NetImguiClient)
add_subdirectory(NetImguiServerApp)
