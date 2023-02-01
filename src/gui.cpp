#include "gui.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define HANDLE_UPDATE(x) if (x) { opts.needs_update = true; }

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

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("rt")) {
            ImGui::SliderInt("spp", &opts.rt.spp_frame, 1, 64);
            ImGui::SliderInt("spp_max", &opts.rt.spp, 1, 1024);
            HANDLE_UPDATE(ImGui::SliderInt("depth", &opts.rt.depth, 1, 8))
            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("flr")) {
            ImGui::Checkbox("line_only", &opts.flr.line_only);
            HANDLE_UPDATE(ImGui::SliderInt("n_aux", &opts.flr.n_aux, 4, 16))
            HANDLE_UPDATE(ImGui::Checkbox("normal", &opts.flr.normal))
            HANDLE_UPDATE(ImGui::Checkbox("positions", &opts.flr.position))
            HANDLE_UPDATE(ImGui::Checkbox("wireframe", &opts.flr.wireframe))
            HANDLE_UPDATE(ImGui::SliderFloat("width", &opts.flr.linewidth, .5f, 5.f))
            ImGui::TreePop();
        }

        ImGui::End();
    }

    // Render dear imgui into screen
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

} // namespace rtnpr
