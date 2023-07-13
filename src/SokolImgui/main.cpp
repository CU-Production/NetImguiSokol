#define SOKOL_IMPL
#define SOKOL_NO_ENTRY
// #define SOKOL_GLCORE33
//#define SOKOL_WGPU
#define SOKOL_GLES3
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include "imgui.h"
#include "util/sokol_imgui.h"

sg_pass_action pass_action{};
sg_buffer vbuf{};
sg_buffer ibuf{};
sg_pipeline pip{};
sg_bindings bind{};

bool show_test_window = true;
bool show_another_window = false;

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
    shd_desc.vs.source = R"(#version 300 es
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aColor;
out vec4 vColor;
void main() {
  gl_Position = vec4(aPosition, 1.0f);
  vColor = vec4(aColor, 1.0f);
}
)";
    shd_desc.fs.source = R"(#version 300 es
precision mediump float;
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
        // When a module is recompiled, ImGui's static context will be empty. Setting it every frame
        // ensures that the context remains set.
        ImGui::SetCurrentContext(ImGui::GetCurrentContext());

        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        static float f = 0.0f;
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::ColorEdit3("clear color", &pass_action.colors[0].clear_value.r);
        if (ImGui::Button("Test Window")) show_test_window ^= 1;
        if (ImGui::Button("Another Window")) show_another_window ^= 1;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("w: %d, h: %d, dpi_scale: %.1f", sapp_width(), sapp_height(), sapp_dpi_scale());
        if (ImGui::Button(sapp_is_fullscreen() ? "Switch to windowed" : "Switch to fullscreen")) {
            sapp_toggle_fullscreen();
        }

        // 2. Show another simple window, this time using an explicit Begin/End pair
        if (show_another_window) {
            ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiCond_FirstUseEver);
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello");
            ImGui::End();
        }

        // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowDemoWindow()
        if (show_test_window) {
            ImGui::SetNextWindowPos(ImVec2(460, 20), ImGuiCond_FirstUseEver);
            ImGui::ShowDemoWindow();
        }
    }

    simgui_render();

    sg_end_pass();
    sg_commit();
}

void cleanup() {
    simgui_shutdown();
    sg_shutdown();
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