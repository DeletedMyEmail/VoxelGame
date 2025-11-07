#include "MenuLayer.h"
#include "Application.h"
#include "Config.h"
#include "GameLayer.h"
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

    ImGui::Text("Load world");
    static char buf[126] = "File Path";
    ImGui::InputText("##pathInput", buf, sizeof(buf));
    if (ImGui::Button("load"))
    {

        if (!loadConfig(buf, gameConfig))
        {
            LOG_ERROR("unable to load config file");
            core::Application::get().stop();
        }
        else
        {
            core::Application::get().removeLayer("MenuLayer");
            core::Application::get().removeLayer("GameLayer");
            core::Application::get().pushLayer<core::Application::BOTTOM, GameLayer>("GameLayer");
        }
    }

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
