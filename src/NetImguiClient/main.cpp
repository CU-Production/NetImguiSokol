#define SOKOL_IMPL
#define SOKOL_NO_ENTRY
#define SOKOL_GLCORE33
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include "imgui.h"
#include "util/sokol_imgui.h"
#include <NetImgui_Api.h>

sg_pass_action pass_action{};
sg_buffer vbuf{};
sg_buffer ibuf{};
sg_pipeline pip{};
sg_bindings bind{};

bool show_test_window = true;
bool show_another_window = false;

//static int sClientPort				= NetImgui::kDefaultClientPort;
static int sClientPort				= 55443;
//static int sServerPort				= NetImgui::kDefaultServerPort;
static int sServerPort				= 55442;
static char sServerHostname[128]	= {"localhost"};
static bool sbShowDemoWindow		= false;
static char zAppName[]              = {"NetImguiClientApp(Sokol)"};

void init() {
    sg_desc desc = {};
    desc.context = sapp_sgcontext();
    desc.logger.func = slog_func;
    sg_setup(&desc);

    simgui_desc_t simgui_desc = {};
    simgui_setup(&simgui_desc);

    const float vertices[] = {
            // positions     color
            -0.5, -0.5, 0.0, 1.0, 0.0, 0.0,
            0.0,  0.5, 0.0, 0.0, 1.0, 0.0,
            0.5, -0.5, 0.0, 0.0, 0.0, 1.0,
    };
    sg_buffer_desc vb_desc = {};
    vb_desc.data = SG_RANGE(vertices);
    vbuf = sg_make_buffer(&vb_desc);

    const int indices[] = { 0, 1, 2,  };
    sg_buffer_desc ib_desc = {};
    ib_desc.type = SG_BUFFERTYPE_INDEXBUFFER;
    ib_desc.data = SG_RANGE(indices);
    ibuf = sg_make_buffer(&ib_desc);

    sg_shader_desc shd_desc = {};
    shd_desc.attrs[0].name = "aPosition";
    shd_desc.attrs[1].name = "aColor";
    shd_desc.vs.source = R"(
#version 330
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aColor;
out vec4 vColor;
void main() {
  gl_Position = vec4(aPosition, 1.0f);
  vColor = vec4(aColor, 1.0f);
}
)";
    shd_desc.fs.source = R"(
#version 330
in vec4 vColor;
out vec4 frag_color;
void main() {
  frag_color = vColor;
  //frag_color = pow(frag_color, vec4(1.0f/2.2f));
}
)";

    sg_shader shd = sg_make_shader(&shd_desc);

    sg_pipeline_desc pip_desc = {};
    pip_desc.shader = shd;
    pip_desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
    pip_desc.index_type = SG_INDEXTYPE_UINT32;
    pip = sg_make_pipeline(&pip_desc);

    bind.vertex_buffers[0] = vbuf;
    bind.index_buffer = ibuf;

    pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;

//    NetImguiInit
    NetImgui::Startup();
//    NetImgui::ConnectFromApp("NetImguiClientApp(Sokol)", NetImgui::kDefaultClientPort);
//    NetImgui::ConnectFromApp("NetImguiClientApp(Sokol)", 55443);
}

void frame() {
    const int width = sapp_width();
    const int height = sapp_height();

    sg_begin_default_pass(&pass_action, width, height);
    sg_apply_pipeline(pip);
    sg_apply_bindings(&bind);
    sg_draw(0, 3, 1);


    simgui_new_frame({ width, height, sapp_frame_duration(), sapp_dpi_scale() });

    {
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3,6) );
            if( ImGui::BeginMainMenuBar() )
            {
                ImGui::AlignTextToFramePadding();
                ImGui::TextColored(ImVec4(0.1, 1, 0.1, 1), "%s", zAppName);
                ImGui::SameLine(0,32);

                //-----------------------------------------------------------------------------------------
                if( NetImgui::IsConnected() )
                    //-----------------------------------------------------------------------------------------
                {
                    ImGui::TextUnformatted("Status: Connected");
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
                    ImGui::SetCursorPosY(3);
                    if( ImGui::Button("Disconnect", ImVec2(120,0)) )
                    {
                        NetImgui::Disconnect();
                    }
                    ImGui::PopStyleVar();
                }

                    //-----------------------------------------------------------------------------------------
                else if( NetImgui::IsConnectionPending() )
                    //-----------------------------------------------------------------------------------------
                {
                    ImGui::TextUnformatted("Status: Waiting Server");
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
                    ImGui::SetCursorPosY(3);
                    if (ImGui::Button("Cancel", ImVec2(120,0)))
                    {
                        NetImgui::Disconnect();
                    }
                    ImGui::PopStyleVar();
                }

                    //-----------------------------------------------------------------------------------------
                else // No connection
                    //-----------------------------------------------------------------------------------------
                {
                    //-------------------------------------------------------------------------------------
                    if( ImGui::BeginMenu("[ Connect to ]") )
                        //-------------------------------------------------------------------------------------
                    {
                        ImGui::TextColored(ImVec4(0.1, 1, 0.1, 1), "Server Settings");
                        ImGui::InputText("Hostname", sServerHostname, sizeof(sServerHostname));
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Address of PC running the netImgui server application. Can be an IP like 127.0.0.1");
                        ImGui::InputInt("Port", &sServerPort);
                        ImGui::NewLine();
                        ImGui::Separator();
                        if (ImGui::Button("Connect", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                        {
                            NetImgui::ConnectToApp(zAppName, sServerHostname, sServerPort);
                        }
                        ImGui::EndMenu();
                    }

                    if( ImGui::IsItemHovered() )
                        ImGui::SetTooltip("Attempt a connection to a remote netImgui server at the provided address.");

                    //-------------------------------------------------------------------------------------
                    if (ImGui::BeginMenu("[  Wait For  ]"))
                        //-------------------------------------------------------------------------------------
                    {
                        ImGui::TextColored(ImVec4(0.1, 1, 0.1, 1), "Client Settings");
                        ImGui::InputInt("Port", &sClientPort);
                        ImGui::NewLine();
                        ImGui::Separator();
                        if (ImGui::Button("Listen", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                        {
                            NetImgui::ConnectFromApp(zAppName, sClientPort);
                        }
                        ImGui::EndMenu();
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Start listening for a connection request by a remote netImgui server, on the provided Port.");
                }

                ImGui::SameLine(0,40);
                ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.8,0.8,0.8,0.9) );
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, sbShowDemoWindow ? 1.f : 0.f);
                ImGui::SetCursorPosY(3);
                if( ImGui::Button("Show ImGui Demo", ImVec2(120,0)) )
                {
                    sbShowDemoWindow = !sbShowDemoWindow;
                }
                ImGui::PopStyleColor();
                ImGui::PopStyleVar(2);
                ImGui::EndMainMenuBar();
            }
            ImGui::PopStyleVar();

            if( sbShowDemoWindow )
            {
                ImGui::ShowDemoWindow(&sbShowDemoWindow);
            }
        }

        {
            ImGui::SetNextWindowPos(ImVec2(32,48), ImGuiCond_Once);
            ImGui::SetNextWindowSize(ImVec2(400,400), ImGuiCond_Once);
            if( ImGui::Begin("Sample Basic", nullptr) )
            {
                ImGui::TextColored(ImVec4(0.1, 1, 0.1, 1), "Basic demonstration of NetImgui code integration.");
                ImGui::TextWrapped("Create a basic Window with some text.");
                ImGui::NewLine();
                ImGui::TextColored(ImVec4(0.1, 1, 0.1, 1), "Where are we drawing: ");
                ImGui::SameLine();
                ImGui::TextUnformatted(NetImgui::IsDrawingRemote() ? "Remote Draw" : "Local Draw");
                ImGui::NewLine();
                ImGui::TextColored(ImVec4(0.1, 1, 0.1, 1), "Filler content");
                ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
            }
            ImGui::End();

            if( ImGui::Begin("Debug Panel", nullptr) )
            {
                static float f = 0.0f;
                ImGui::Text("Hello, world!");
                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
                ImGui::ColorEdit3("clear color", &pass_action.colors[0].clear_value.r);
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::Text("w: %d, h: %d, dpi_scale: %.1f", sapp_width(), sapp_height(), sapp_dpi_scale());
                if (ImGui::Button(sapp_is_fullscreen() ? "Switch to windowed" : "Switch to fullscreen")) {
                    sapp_toggle_fullscreen();
                }
            }
            ImGui::End();
        }
    }

    ImGui::Render();
    if (!NetImgui::IsConnected())
    {
        ImDrawData* draw_data = ImGui::GetDrawData();
        simgui_render_draw_data(draw_data);
    }

    sg_end_pass();
    sg_commit();
}

void cleanup() {
    simgui_shutdown();
    sg_shutdown();

    NetImgui::Shutdown();
}

void input(const sapp_event* event) {
    simgui_handle_event(event);
}

int main(int argc, const char* argv[]) {
    sapp_desc desc = {};
    desc.init_cb = init;
    desc.frame_cb = frame;
    desc.cleanup_cb = cleanup,
    desc.event_cb = input,
    desc.width = 800,
    desc.height = 600,
    desc.window_title = "triangle",
    desc.icon.sokol_default = true,
    desc.logger.func = slog_func;
    sapp_run(desc);

    return 0;
}
