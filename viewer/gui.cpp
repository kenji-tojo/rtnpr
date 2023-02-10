#include "gui.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


namespace viewer {

void CheckBox::draw() {
    if (ImGui::Checkbox(label, &enabled)) { if (on_update) on_update(); }
}

void IntSlider::draw() {
    if (ImGui::SliderInt(label, &val, min, max)) { if (on_update) on_update(); }
}

void FloatSlider::draw() {
    if (ImGui::SliderFloat(label, &val, min, max)) { if (on_update) on_update(); }
}

void Button::draw() {
    if (ImGui::Button(label)) { if (on_update) on_update(); }
}


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

    // GUI contents
    {
        ImGui::Begin("GUI");

        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);

        for (auto &e: top_level.elements) { e->draw(); }

        for (auto &tn: tree_nodes) {
            ImGui::SetNextItemOpen(tn.open, ImGuiCond_Once);
            if (ImGui::TreeNode(tn.label)) {
                for (auto &e: tn.elements) { e->draw(); }
                ImGui::TreePop();
            }
        }

        ImGui::End();
    }

    // Render dear imgui into screen
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

} // namespace viewer
