#include "DebugLayer.h"

#include <ranges>

#include "Application.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "OpenGLHelper.h"
#include "../cmake-build-release/_deps/implot-src/implot.h"

VertexArray createAxesVAO();

DebugLayer::DebugLayer(const std::string& name)
    :   Layer(name),
        m_AxisVao(createAxesVAO()),
        m_Shader("../resources/shaders/BasicVert.glsl", "../resources/shaders/BasicFrag.glsl")
{
    m_GameLayer = (GameLayer*)core::Application::get().getLayer("GameLayer");
    assert(m_GameLayer);
}

void DebugLayer::onUpdate(const double dt)
{

}

void DebugLayer::onRender()
{
    drawMenu();
    drawAxes();
}

bool DebugLayer::onEvent(core::Event& e)
{
    switch (e.type)
    {
        case core::EventType::KeyPressed:
            if (e.keyEvent.key == GLFW_KEY_F3)
            {
                LOG_INFO("Suspending DebugLayer");
                m_Enabled = false;
                return true;
            }
            return false;
        case core::EventType::CursorMoved:
            ImGui::GetIO().MousePos = ImVec2(float(e.cursorEvent.pos.x), float(e.cursorEvent.pos.y));
            return false;
        case core::EventType::MouseButtonPressed:
            ImGui::GetIO().MouseDown[e.mouseEvent.button] = true;
            return false;
        case core::EventType::MouseButtonReleased:
            ImGui::GetIO().MouseDown[e.mouseEvent.button] = false;
            return false;
        default: return false;
    }
}

VertexArray createAxesVAO()
{
    const float axisVertices[] =
    {
        // X axis
        0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0,
        0.5f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0,

        // Y axis
        0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0,
        0.0f, 0.5f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0,

        // Z axis
        0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, 1.0,
        0.0f, 0.0f, 0.5f,   0.0f, 0.0f, 1.0f, 1.0
    };

    const GLuint vbo = createBuffer(axisVertices, sizeof(axisVertices));
    VertexBufferLayout layout;
    layout.pushFloat(3);
    layout.pushFloat(4);
    VertexArray vao;
    vao.addBuffer(vbo, layout);
    vao.vertexCount = 6;
    return vao;
}

void DebugLayer::drawMenu() const
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Debug");

    const auto& pos = m_GameLayer->m_PlayerPhysics.box.pos;
    ImGui::Text("Position: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
    ImGui::Spacing();ImGui::Spacing();

    ImGui::Text("Seed: %d", gameConfig.worldSeed);
    ImGui::Text("Render Distance: %d", gameConfig.renderDistance);
    ImGui::Text("Load Distance: %d", gameConfig.loadDistance);
    ImGui::Text("Threads: %d", gameConfig.threadCount);
    ImGui::Spacing();ImGui::Spacing();

    ImGui::Checkbox("Player Physics", &m_GameLayer->playerPhysicsOn);
    ImGui::SliderFloat("Exposure", &m_GameLayer->exposure, 0.0f, 1.0f);
    ImGui::SliderFloat("Camera Speed", &m_GameLayer->camSpeed, 1.0f, 200.0f);
    ImGui::Combo("Block Type", (int*) &m_GameLayer->selectedBlock, BLOCK_NAMES.data(), BLOCK_NAMES.size());
    ImGui::Spacing();ImGui::Spacing();

    const auto& metrics = m_GameLayer->m_Metrics;
    ImGui::Text("Frame data for last %.1f seconds:", metrics.interval);
    const double avgFrameTime = metrics.getAvgFrameTime();
    ImGui::Text("Avg frame time: %.3f ms (%.1f FPS)", avgFrameTime * 1000.0, 1.0f / avgFrameTime);
    const double maxFrameTime = metrics.get1PercentLowFrameTime();
    ImGui::Text("1%% lows: %.3f ms (%.1f FPS)", maxFrameTime * 1000.0, 1.0f / maxFrameTime);
    ImGui::Spacing();

    if (ImPlot::BeginPlot("Frame Times"))
    {
        ImPlot::SetupAxes("Sample", "Frame Time (ms)");
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, maxFrameTime * 2000, ImGuiCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, metrics.frameTimes.size(), ImGuiCond_Always);

        // Convert frame times from seconds to milliseconds for plotting
        std::vector<float> frameTimesMs;
        frameTimesMs.reserve(metrics.frameTimes.size());
        for (const auto& frameTime : metrics.frameTimes) {
            frameTimesMs.push_back(static_cast<float>(frameTime * 1000.0));
        }

        if (!frameTimesMs.empty()) {
            ImPlot::PlotLine("Frame Time", frameTimesMs.data(), frameTimesMs.size());
        }

        ImPlot::EndPlot();
    }

    for (const auto& [name, time] : metrics.timer)
        ImGui::Text("%s: %.3f ms", name.c_str(), time * 1000.0);

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DebugLayer::drawAxes()
{
    m_AxisVao.bind();
    m_Shader.bind();

    m_Shader.setUniformMat4("u_VP", m_GameLayer->m_Cam.viewProjection);
    m_Shader.setUniform3f("u_GlobalPosition", m_GameLayer->m_Cam.position + m_GameLayer->m_Cam.lookDir);

    GLCall(glDrawArrays(GL_LINES, 0, m_AxisVao.vertexCount));
}
