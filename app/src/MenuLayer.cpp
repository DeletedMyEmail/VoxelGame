#include "MenuLayer.h"
#include "Application.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

MenuLayer::MenuLayer(const std::string& name): Layer(name)
{}

bool MenuLayer::onEvent(core::Event& e)
{
    return false;
}

void MenuLayer::onRender()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const ImGuiIO& io = ImGui::GetIO();
    const ImVec2 pos(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin("Menu", nullptr, flags);

    if (ImGui::Button("Quit"))
        core::Application::get().stop();

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void MenuLayer::onAttach()
{
    core::Application::get().getWindow().disableCursor(false);
}

void MenuLayer::onDetach()
{
    core::Application::get().getWindow().disableCursor();
}
