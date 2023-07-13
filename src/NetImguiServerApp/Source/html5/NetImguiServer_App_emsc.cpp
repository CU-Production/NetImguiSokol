//=================================================================================================
// @SAMPLE_EDIT
// Note:    This file a duplicate of 'Dear ImGui Sample' : "examples\example_glfw_opengl3\main.cpp"
//          With a few editions added to customize it for our NetImGui server needs. 
//          These fews edits will be found in a few location, using the tag '@SAMPLE_EDIT' 
#include "NetImguiServer_App.h"

#if HAL_API_PLATFORM_HTML5

#include <string>
#include "NetImguiServer_UI.h"

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#define SOKOL_IMPL
#define SOKOL_GLES3
#include "sokol_gfx.h"
#include "sokol_time.h"
#include "sokol_log.h"
#include "emsc.h"
#include "imgui.h"
#define SOKOL_IMGUI_NO_SOKOL_APP
#include "util/sokol_imgui.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/websocket.h>
#include <emscripten/threading.h>
#include <emscripten/posix_socket.h>

static EMSCRIPTEN_WEBSOCKET_T bridgeSocket = 0;
#endif

static uint64_t last_time = 0;

static sg_pass_action pass_action;
static bool btn_down[3];
static bool btn_up[3];

static EM_BOOL draw(double time, void* userdata);

int main() {
#ifdef __EMSCRIPTEN__
    bridgeSocket = emscripten_init_websocket_to_posix_socket_bridge("ws://localhost:9999");
    // Synchronously wait until connection has been established.
    uint16_t readyState = 0;
    do {
        emscripten_websocket_get_ready_state(bridgeSocket, &readyState);
        emscripten_thread_sleep(100);
    } while (readyState == 0);
#endif

    /* setup WebGL context */
    emsc_init("#canvas", EMSC_NONE);

    /* setup sokol_gfx and sokol_time */
    stm_setup();
    sg_desc desc{};
    desc.logger.func = slog_func;
    desc.context.depth_format = SG_PIXELFORMAT_NONE;
    desc.disable_validation = true;
    sg_setup(desc);
    assert(sg_isvalid());

    simgui_desc_t simgui_desc = {};
    simgui_desc.no_default_font = true;
    simgui_desc.depth_format = SG_PIXELFORMAT_NONE;
    simgui_setup(&simgui_desc);

    pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    bool ok = NetImguiServer::App::Startup("");
    IM_ASSERT(ok);

    // setup the ImGui environment
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    io.IniFilename = nullptr;
    io.Fonts->AddFontDefault();
    // emscripten has no clearly defined key code constants
    io.KeyMap[ImGuiKey_Tab] = 9;
    io.KeyMap[ImGuiKey_LeftArrow] = 37;
    io.KeyMap[ImGuiKey_RightArrow] = 39;
    io.KeyMap[ImGuiKey_UpArrow] = 38;
    io.KeyMap[ImGuiKey_DownArrow] = 40;
    io.KeyMap[ImGuiKey_Home] = 36;
    io.KeyMap[ImGuiKey_End] = 35;
    io.KeyMap[ImGuiKey_Delete] = 46;
    io.KeyMap[ImGuiKey_Backspace] = 8;
    io.KeyMap[ImGuiKey_Enter] = 13;
    io.KeyMap[ImGuiKey_Escape] = 27;
    io.KeyMap[ImGuiKey_A] = 65;
    io.KeyMap[ImGuiKey_C] = 67;
    io.KeyMap[ImGuiKey_V] = 86;
    io.KeyMap[ImGuiKey_X] = 88;
    io.KeyMap[ImGuiKey_Y] = 89;
    io.KeyMap[ImGuiKey_Z] = 90;

    //    // IMGUI Font texture init
    {
        ImGuiIO& io = ImGui::GetIO();

        // Build texture atlas
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

        sg_image_desc img_desc{};
        img_desc.width = width;
        img_desc.height = height;
        img_desc.label = "font-texture";
        img_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
        img_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
        img_desc.min_filter = SG_FILTER_LINEAR;
        img_desc.mag_filter = SG_FILTER_LINEAR;
        img_desc.data.subimage[0][0].ptr = pixels;
        img_desc.data.subimage[0][0].size = (width * height) * sizeof(uint32_t);

        sg_image img = sg_make_image(&img_desc);

        io.Fonts->TexID = (ImTextureID)(uintptr_t)img.id;
    }

    // emscripten to ImGui input forwarding
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true,
                                    [](int, const EmscriptenKeyboardEvent* e, void*)->EM_BOOL {
                                        if (e->keyCode < 512) {
                                            ImGui::GetIO().KeysDown[e->keyCode] = true;
                                        }
                                        // only forward alpha-numeric keys to browser
                                        return e->keyCode < 32;
                                    });
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true,
                                  [](int, const EmscriptenKeyboardEvent* e, void*)->EM_BOOL {
                                      if (e->keyCode < 512) {
                                          ImGui::GetIO().KeysDown[e->keyCode] = false;
                                      }
                                      // only forward alpha-numeric keys to browser
                                      return e->keyCode < 32;
                                  });
    emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true,
                                     [](int, const EmscriptenKeyboardEvent* e, void*)->EM_BOOL {
                                         ImGui::GetIO().AddInputCharacter((ImWchar)e->charCode);
                                         return true;
                                     });
    emscripten_set_mousedown_callback("canvas", nullptr, true,
                                      [](int, const EmscriptenMouseEvent* e, void*)->EM_BOOL {
                                          switch (e->button) {
                                              case 0: btn_down[0] = true; break;
                                              case 2: btn_down[1] = true; break;
                                          }
                                          return true;
                                      });
    emscripten_set_mouseup_callback("canvas", nullptr, true,
                                    [](int, const EmscriptenMouseEvent* e, void*)->EM_BOOL {
                                        switch (e->button) {
                                            case 0: btn_up[0] = true; break;
                                            case 2: btn_up[1] = true; break;
                                        }
                                        return true;
                                    });
    emscripten_set_mouseenter_callback("canvas", nullptr, true,
                                       [](int, const EmscriptenMouseEvent*, void*)->EM_BOOL {
                                           auto& io = ImGui::GetIO();
                                           for (int i = 0; i < 3; i++) {
                                               btn_down[i] = btn_up[i] = false;
                                               io.MouseDown[i] = false;
                                           }
                                           return true;
                                       });
    emscripten_set_mouseleave_callback("canvas", nullptr, true,
                                       [](int, const EmscriptenMouseEvent*, void*)->EM_BOOL {
                                           auto& io = ImGui::GetIO();
                                           for (int i = 0; i < 3; i++) {
                                               btn_down[i] = btn_up[i] = false;
                                               io.MouseDown[i] = false;
                                           }
                                           return true;
                                       });
    emscripten_set_mousemove_callback("canvas", nullptr, true,
                                      [](int, const EmscriptenMouseEvent* e, void*)->EM_BOOL {
                                          ImGui::GetIO().MousePos.x = (float) e->targetX;
                                          ImGui::GetIO().MousePos.y = (float) e->targetY;
                                          return true;
                                      });
    emscripten_set_wheel_callback("canvas", nullptr, true,
                                  [](int, const EmscriptenWheelEvent* e, void*)->EM_BOOL {
                                      ImGui::GetIO().MouseWheelH = -0.1f * (float)e->deltaX;
                                      ImGui::GetIO().MouseWheel = -0.1f * (float)e->deltaY;
                                      return true;
                                  });

    // initial clear color
    pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;

    emscripten_request_animation_frame_loop(draw, 0);
    return 0;
}

static EM_BOOL draw(double time, void* userdata) {
    const int width = emsc_width();
    const int height = emsc_height();

    simgui_new_frame({ width, height, (float)stm_sec(stm_laptime(&last_time)), 1.0f });

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(float(emsc_width()), float(emsc_height()));
    io.DeltaTime = (float) stm_sec(stm_laptime(&last_time));
    // this mouse button handling fixes the problem when down- and up-events
    // happen in the same frame
    for (int i = 0; i < 3; i++) {
        if (io.MouseDown[i]) {
            if (btn_up[i]) {
                io.MouseDown[i] = false;
                btn_up[i] = false;
            }
        }
        else {
            if (btn_down[i]) {
                io.MouseDown[i] = true;
                btn_down[i] = false;
            }
        }
    }

    NetImguiServer::App::UpdateRemoteContent(); // @SAMPLE_EDIT (Request each client to update their drawing content )
    clear_color = NetImguiServer::UI::DrawImguiContent();
    pass_action.colors[0].clear_value.r = clear_color.x;
    pass_action.colors[0].clear_value.g = clear_color.y;
    pass_action.colors[0].clear_value.b = clear_color.z;
    pass_action.colors[0].clear_value.a = clear_color.w;

    sg_begin_default_pass(&pass_action, width, height);
    simgui_render();
    sg_end_pass();

    sg_commit();
    return EM_TRUE;
}


#endif // @SAMPLE_EDIT HAL_API_PLATFORM_HTML5
