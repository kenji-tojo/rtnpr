#include "gui.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define NEEDS_UPDATE(x) if (x) { opts.needs_update = true; }

namespace rtnpr {

Gui::Gui(GLFWwindow *window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true); // Setup Platform/Renderer bindings
    ImGui::StyleColorsClassic();
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

Gui::~Gui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Gui::draw()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    opts.needs_update = false;

    // GUI contents
    {
        ImGui::Begin("GUI");

        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);

        if (ImGui::TreeNode("rt")) {
            ImGui::SliderInt("spp", &opts.rt.spp_frame, 1, 64);
            ImGui::SliderInt("spp_max", &opts.rt.spp, 1, 1024);
            NEEDS_UPDATE(ImGui::SliderInt("depth", &opts.rt.depth, 1, 8))
            static float back_brightness = 1.f;
            ImGui::SliderFloat("back_brightness", &back_brightness, 0.f, 1.f);
            opts.rt.back_color = Eigen::Vector3f{1.f,1.f,1.f} * back_brightness;
            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("flr")) {
            ImGui::Checkbox("line_only", &opts.flr.line_only);
            NEEDS_UPDATE(ImGui::SliderInt("n_aux", &opts.flr.n_aux, 4, 16))
            NEEDS_UPDATE(ImGui::Checkbox("normal", &opts.flr.normal))
            NEEDS_UPDATE(ImGui::Checkbox("positions", &opts.flr.position))
            NEEDS_UPDATE(ImGui::Checkbox("wireframe", &opts.flr.wireframe))
            NEEDS_UPDATE(ImGui::SliderFloat("width", &opts.flr.linewidth, .5f, 5.f))
            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("ground")) {
            NEEDS_UPDATE(ImGui::Checkbox("visible", &opts.scene.plane->visible));
            NEEDS_UPDATE(ImGui::SliderInt("mat_id", &opts.scene.plane->mat_id, 1, 3))
            NEEDS_UPDATE(ImGui::Checkbox("checkerboard", &opts.scene.plane->checkerboard))
            NEEDS_UPDATE(ImGui::SliderInt("check_res", &opts.scene.plane->check_res, 5, 50))
            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("tone")) {
            static int map_mode = 1;
            if (ImGui::SliderInt("map_mode", &map_mode, 0, 2)) {
                opts.tone.map_mode = ToneMapper::MapMode(map_mode);
                opts.needs_update = true;
            }
            NEEDS_UPDATE(ImGui::Checkbox("map_lines", &opts.tone.map_lines))
            ImGui::Checkbox("map_shading", &opts.tone.map_shading);
            {
                using namespace Eigen;
                if (opts.tone.map_shading) {
                    opts.tone.mapper.hi_rgb = Vector3f{250.f/255.f,210.f/255.f,219.f/255.f};
                    opts.tone.mapper.lo_rgb = Vector3f{165.f/255.f,206.f/255.f,239.f/255.f};
                }
                else {
                    opts.tone.mapper.hi_rgb = Vector3f::Ones();
                    opts.tone.mapper.lo_rgb = Vector3f::Zero();
                }
            }
            ImGui::TreePop();
        }

        ImGui::End();
    }

    // Render dear imgui into screen
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

} // namespace rtnpr
