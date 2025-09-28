#include "../include/Rendering.h"
#include <algorithm>
#include <numeric>
#include "Block.h"
#include "Config.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "OpenGLHelper.h"
#include "Shader.h"
#include "Texture.h"

VertexArray createAxesVAO();
VertexArray createHighlightVAO();

Renderer::Renderer(GLFWwindow* win)
    :   m_AxisVao(createAxesVAO()),
        m_HighlightVao(createHighlightVAO()),
        m_TextureAtlas("../resources/textures/TextureAtlas.png"),
        m_BasicShader("../resources/shaders/BasicVert.glsl", "../resources/shaders/BasicFrag.glsl"),
        m_BlockShader("../resources/shaders/BlockVert.glsl", "../resources/shaders/BlockFrag.glsl")
{
    GLCall(glEnable(GL_CULL_FACE));
    GLCall(glCullFace(GL_FRONT));
    GLCall(glEnable(GL_DEPTH_TEST));
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(win, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

Renderer::~Renderer()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Renderer::drawEntity(const VertexArray& vao, const glm::vec3& pos, const glm::mat4& viewProjection, const float exposure)
{
    vao.bind();
    m_BasicShader.bind();

    m_BasicShader.setUniformMat4("u_VP", viewProjection);
    m_BasicShader.setUniform3f("u_GlobalPosition", pos);
    GLCall(glDrawArrays(GL_LINES, 0, vao.vertexCount));
}

void Renderer::prepareChunkRendering(const glm::mat4& viewProjection, const float exposure)
{
    m_TextureAtlas.bind(0);
    m_BlockShader.bind();
    m_BlockShader.setUniformMat4("u_VP", viewProjection);
    m_BlockShader.setUniform1i("u_textureSlot", 0);
    m_BlockShader.setUniform3f("u_exposure", glm::vec3{exposure});
}

void Renderer::drawChunk(const VertexArray& vao, const glm::ivec3& globalOffset)
{
    if (vao.vertexCount == 0)
        return;

    m_BlockShader.setUniform3f("u_chunkOffset", glm::vec3(globalOffset));
    vao.bind();
    GLCall(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, vao.vertexCount));
}

void Renderer::drawAxes(const Camera& cam)
{
    m_AxisVao.bind();
    m_BasicShader.bind();

    m_BasicShader.setUniformMat4("u_VP", cam.viewProjection);
    m_BasicShader.setUniform3f("u_GlobalPosition", cam.position + cam.lookDir);

    GLCall(glDrawArrays(GL_LINES, 0, m_AxisVao.vertexCount));
}

void Renderer::drawHighlightBlock(const glm::vec3& pos, const glm::mat4& viewProjection, const float exposure)
{
    GLCall(glEnable(GL_DEPTH_TEST));
    GLCall(glDepthFunc(GL_LEQUAL));

    m_HighlightVao.bind();
    m_TextureAtlas.bind(0);
    m_BlockShader.bind();

    m_BlockShader.setUniformMat4("u_VP", viewProjection);
    m_BlockShader.setUniform1i("u_textureSlot", 0);
    m_BlockShader.setUniform3f("u_chunkOffset", pos);
    m_BlockShader.setUniform3f("u_exposure", glm::vec3(exposure));

    GLCall(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, m_HighlightVao.vertexCount));
    GLCall(glDepthFunc(GL_LESS));
}

void Renderer::clearFrame(const float skyExposure, const bool debugMode) const
{
    glClearColor(0.5f * skyExposure, 0.8f * skyExposure, 0.9f * skyExposure, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (debugMode)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
}

void Renderer::drawDebugMenu(const Metrics& metrics, MenuSettings& settings, const glm::vec3& pos, const GameConfig& config) const
{
    ImGui::Begin("Debug");

    ImGui::Text("Position: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
    ImGui::Spacing();ImGui::Spacing();

    ImGui::Text("Seed: %d", config.worldSeed);
    ImGui::Text("Render Distance: %d", config.renderDistance);
    ImGui::Text("Load Distance: %d", config.loadDistance);
    ImGui::Text("Threads: %d", config.threadCount);
    ImGui::Spacing();ImGui::Spacing();

    ImGui::Checkbox("Player Physics", &settings.playerPhysicsOn);
    ImGui::SliderFloat("Exposure", &settings.exposure, 0.0f, 1.0f);
    ImGui::SliderFloat("Camera Speed", &settings.camSpeed, 1.0f, 200.0f);
    ImGui::Combo("Block Type", (int*) &settings.selectedBlock, BLOCK_NAMES.data(), BLOCK_NAMES.size());
    ImGui::Spacing();ImGui::Spacing();

    ImGui::Text("Frame data for last %.1f seconds:", metrics.frameTimeWindow);
    const double avgFrameTime = metrics.getAvgFrameTime();
    ImGui::Text("Avg frame time: %.3f ms (%.1f FPS)", avgFrameTime * 1000.0, 1.0f / avgFrameTime);
    const double maxFrameTime = metrics.get1PercentLowFrameTime();
    ImGui::Text("1%% lows: %.3f ms (%.1f FPS)", maxFrameTime * 1000.0, 1.0f / maxFrameTime);
    ImGui::Spacing();

    for (const auto& [name, time] : metrics.timer)
        ImGui::Text("%s: %.3f ms", name.c_str(), time * 1000.0);

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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

    GLuint vbo = createBuffer(axisVertices, sizeof(axisVertices));
    VertexBufferLayout layout;
    layout.pushFloat(3);
    layout.pushFloat(4);
    VertexArray vao;
    vao.addBuffer(vbo, layout);
    vao.vertexCount = 6;
    return vao;
}


VertexArray createHighlightVAO()
{
    static std::array<blockdata, 6> buffer;
    const glm::uvec2 atlasOffset = getAtlasOffset(BLOCK_TYPE::HIGHLIGHTED, (FACE) 0);

    for (uint32_t i = 0; i < buffer.size(); i++)
        buffer[i] = packBlockData(glm::uvec3(0), atlasOffset, (FACE) i);

    VertexArray highlightVao;
    VertexBufferLayout highlightLayout;
    highlightLayout.pushUInt(1, false, 1);
    highlightVao.addBuffer(createBuffer(buffer.data(), sizeof(blockdata) * buffer.size()), highlightLayout);
    highlightVao.vertexCount = buffer.size();

    return highlightVao;
}