#include "DebugLayer.h"

#include <numeric>
#include <ranges>
#include "Application.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "OpenGLHelper.h"
#include "implot.h"

VertexArray createAxesVAO();

DebugLayer::DebugLayer(const std::string& name)
    :   Layer(name),
        m_AxisVao(createAxesVAO()),
        m_Shader("../resources/shaders/BasicVert.glsl", "../resources/shaders/BasicFrag.glsl")
{
}

void DebugLayer::onUpdate(const double dt)
{
    if (m_FrameTimes.size() > SAMPLE_COUNT)
        m_FrameTimes.pop_front();
    m_FrameTimes.push_back(dt * 1000.0);
}

void DebugLayer::onRender()
{
    const auto gameLayer = (GameLayer*)core::Application::get().getLayer("GameLayer");
    assert(gameLayer);
    
    drawMenu(gameLayer);
    drawAxes(gameLayer);
}

bool DebugLayer::onEvent(core::Event& e)
{
    switch (e.type)
    {
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

void DebugLayer::drawMenu(GameLayer* gameLayer) const
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Debug");

    const auto& pos = gameLayer->m_PlayerPhysics.box.pos;
    ImGui::Text("Position: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
    ImGui::Spacing();ImGui::Spacing();

    const GameConfig gameConfig = gameLayer->m_GameConfig;
    ImGui::Text("Seed: %d", gameConfig.worldSeed);
    ImGui::Text("Render Distance: %d", gameConfig.renderDistance);
    ImGui::Text("Load Distance: %d", gameConfig.loadDistance);
    ImGui::Text("Threads: %d", gameConfig.threadCount);
    ImGui::Spacing();ImGui::Spacing();

    ImGui::Checkbox("Player Physics", &gameLayer->m_PlayerPhysicsOn);
    ImGui::SliderFloat("Exposure", &gameLayer->m_Exposure, 0.0f, 1.0f);
    ImGui::SliderFloat("Camera Speed", &gameLayer->m_CamSpeed, 1.0f, 200.0f);
    ImGui::Combo("Block Type", (int*) &gameLayer->selectedBlock, BLOCK_NAMES.data(), BLOCK_NAMES.size());
    ImGui::Spacing();ImGui::Spacing();

#ifndef NOPROFILE

    if (!ImPlot::BeginPlot("Profiling metrics"))
    {
        LOG_ERROR("Failed to begin plot");
    }
    else
    {
        ImPlot::SetupAxes("Sample", "Frame Time (ms)");

        double maxPlotY = 0;
        for (auto& data : metrics | std::views::values)
        {
            for (auto& val : data.values)
                maxPlotY = std::max(maxPlotY, val);
        }

        ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, maxPlotY, ImGuiCond_Always);
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, SAMPLE_COUNT, ImGuiCond_Always);

        for (auto& [name, data] : metrics)
        {
            std::array<double, SAMPLE_COUNT> frameTimesMs{};
            size_t i = 0;
            for (const auto& val : data.values)
                frameTimesMs[i++] = val;

            ImPlot::PushStyleColor(ImPlotCol_Line, data.color);
            ImPlot::PlotLine(name.c_str(), frameTimesMs.data(), i);
            ImPlot::PopStyleColor();
        }

        ImPlot::EndPlot();
        const double avgFrameTimeMs = std::accumulate(m_FrameTimes.begin(), m_FrameTimes.end(), 0.0) / m_FrameTimes.size();
        ImGui::Text("Avg frame time: %.2f ms", avgFrameTimeMs);
    }
#endif

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DebugLayer::drawAxes(const GameLayer* gameLayer)
{
    m_AxisVao.bind();
    m_Shader.bind();

    m_Shader.setUniformMat4("u_VP", gameLayer->m_Cam.viewProjection);
    m_Shader.setUniform3f("u_GlobalPosition", gameLayer->m_Cam.position + gameLayer->m_Cam.lookDir);

    GLCall(glDrawArrays(GL_LINES, 0, m_AxisVao.vertexCount));
}
