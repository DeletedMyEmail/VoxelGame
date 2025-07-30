#include "Rendering.h"
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

void drawChunk(const VertexArray& vao, const glm::ivec3& globalOffset, const glm::mat4& viewProjection, const float exposure)
{
    vao.bind();

    static Texture textureAtlas("../resources/textures/TextureAtlas.png");
    textureAtlas.bind(0);

    static GLuint blockShader = createShader("../resources/shaders/BlockVert.glsl", "../resources/shaders/BlockFrag.glsl");
    bind(blockShader);

    setUniformMat4(blockShader, "u_VP", viewProjection);
    setUniform1i(blockShader, "u_textureSlot", 0);
    setUniform3f(blockShader, "u_exposure", glm::vec3{exposure});
    setUniform3f(blockShader, "u_chunkOffset", glm::vec3(globalOffset));

    GLCall(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, vao.vertexCount / 6));
}

void drawAxes(const Camera& cam)
{
    static VertexArray axisVbo = createAxesVAO();
    axisVbo.bind();

    static auto basicShader = createShader("../resources/shaders/BasicVert.glsl", "../resources/shaders/BasicFrag.glsl");
    bind(basicShader);

    setUniformMat4(basicShader, "u_VP", cam.viewProjection);
    setUniform3f(basicShader, "u_GlobalPosition", cam.position + cam.lookDir);

    GLCall(glDrawArrays(GL_LINES, 0, axisVbo.vertexCount));
}

void drawHighlightBlock(const glm::uvec3& positionInChunk, const glm::ivec3& globalOffset, const glm::mat4& viewProjection, const float exposure)
{
    GLCall(glEnable(GL_DEPTH_TEST));
    GLCall(glDepthFunc(GL_LEQUAL));

    static std::array<blockdata, 6> buffer;
    const glm::uvec2 atlasOffset = getAtlasOffset(BLOCK_TYPE::HIGHLIGHTED, FACE(0));

    for (uint32_t i = 0; i < buffer.size(); i++)
    {
        const blockdata packedData =
            ((i & FACE_MASK) << FACE_OFFSET) |
            ((positionInChunk.x & XPOS_MASK) << XPOS_OFFSET) |
            ((positionInChunk.y & YPOS_MASK) << YPOS_OFFSET) |
            ((positionInChunk.z & ZPOS_MASK) << ZPOS_OFFSET) |
            ((atlasOffset.x & ATLASX_MASK) << ATLASX_OFFSET) |
            ((atlasOffset.y & ATLASY_MASK) << ATLASY_OFFSET);

        buffer[i] = packedData;
    }

    VertexArray highlightVao;
    VertexBufferLayout highlightLayout;
    highlightLayout.pushUInt(1, false, 1);
    highlightVao.addBuffer(createBuffer(buffer.data(), sizeof(blockdata) * buffer.size()), highlightLayout);
    highlightVao.bind();

    static Texture textureAtlas("../resources/textures/TextureAtlas.png");
    textureAtlas.bind(0);

    static GLuint blockShader = createShader("../resources/shaders/BlockVert.glsl", "../resources/shaders/BlockFrag.glsl");
    bind(blockShader);

    setUniformMat4(blockShader, "u_VP", viewProjection);
    setUniform1i(blockShader, "u_textureSlot", 0);
    setUniform3f(blockShader, "u_chunkOffset", glm::vec3(globalOffset));
    setUniform3f(blockShader, "u_exposure", glm::vec3{exposure});

    GLCall(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, buffer.size()));
    GLCall(glDepthFunc(GL_LESS));
}

void clearFrame(const float skyExposure, const bool debugMode)
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

void drawDebugMenu(const Metrics& metrics, float& exposure, float& camSpeed, const Camera& cam, const std::array<const char*, 5>& comboSelection, int32_t& comboIndex)
{
    ImGui::Begin("Debug");

    ImGui::Text("Frame data for last %.1f seconds:", metrics.frameTimeWindow);
    const float avgFrameTime = metrics.getAvgFrameTime();
    ImGui::Text("Avg frame time: %.3f ms (%.1f FPS)", avgFrameTime * 1000.0f, 1.0f / avgFrameTime);
    float maxFrameTime = metrics.get1PercentLowFrameTime();
    ImGui::Text("1%% lows: %.3f ms (%.1f FPS)", maxFrameTime * 1000.0f, 1.0f / maxFrameTime);
    ImGui::Spacing();ImGui::Spacing();

    ImGui::Text("Camera Position: %.2f, %.2f, %.2f", cam.position.x, cam.position.y, cam.position.z);
    ImGui::Spacing();ImGui::Spacing();

    ImGui::Text("Seed: %d", config::WORLD_SEED);
    ImGui::Spacing();ImGui::Spacing();

    ImGui::SliderFloat("Exposure", &exposure, 0.0f, 1.0f);
    ImGui::SliderFloat("Camera Speed", &camSpeed, 10.0f, 1000.0f);
    ImGui::Combo("Block", &comboIndex, comboSelection.data(), comboSelection.size());
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void renderCleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}

void renderConfig(GLFWwindow* window)
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
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

    GLuint vbo = createBuffer(axisVertices, 6 * 7 * sizeof(float));
    VertexBufferLayout layout;
    layout.pushFloat(3);
    layout.pushFloat(4);
    VertexArray vao;
    vao.addBuffer(vbo, layout);
    vao.vertexCount = 6;
    return vao;
}