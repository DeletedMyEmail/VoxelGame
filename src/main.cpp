#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include <cstmlib/Profiling.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Camera.h"
#include "Chunk.h"
#include "Shader.h"
#include "Window.h"
#include "OpenGLHelper.h"
#include "Raycast.h"
#include "Texture.h"
#include "ThreadPool.h"
#include <deque>
#include <numeric>
#include <algorithm>

VertexArray createAxesVAO();
glm::vec3 rawInput(const Window& window);
void drawHighlightBlock(const glm::vec3& worldPos, const glm::uvec2& chunkPos, GLuint shader);

int main(int argc, char* argv[])
{
    LOG_INIT();
    PROFILER_INIT(100);

    bool debugMode = true;
    bool cursorLocked = true;

    Window window;
    Camera cam(glm::vec3{1000, Chunk::CHUNK_SIZE, 1000}, 90.0f, window.getWidth(), window.getHeight(), 0.1f, config::RENDER_DISTANCE * Chunk::CHUNK_SIZE * 4);
    float camSpeed = 70.0f;

    ChunkManager chunkManager;

    std::array<const char*, 5> comboSelection{ "None", "Stone", "Grass", "Sand", "Wood"};
    int32_t comboIndex = 0;

#pragma region window
    glm::dvec2 prevCursorPos = window.getMousePosition();

    window.onKey([&chunkManager, &debugMode, &cursorLocked] (Window* win, const int key, const int scancode, const int action, const int mods)
    {
        if (action != GLFW_PRESS)
            return;
        if (key == GLFW_KEY_ESCAPE)
            win->stop();
        else if (key == GLFW_KEY_TAB)
            debugMode = !debugMode;
        else if (key == GLFW_KEY_V)
        {
            cursorLocked = !cursorLocked;
            win->setCursorDisabled(cursorLocked);
        }
        else if (key == GLFW_KEY_X)
            chunkManager.dropChunkMeshes();

    });
    window.onCursorMove([&cam, &prevCursorPos, &cursorLocked](Window* win, const glm::dvec2 pos)
        {
            const glm::dvec2 offset = pos - prevCursorPos;
            if (cursorLocked)
                cam.rotate({offset.x, offset.y});
            prevCursorPos = pos;
        });

    window.onMouseButton([&cam, &chunkManager, &cursorLocked, &comboSelection, &comboIndex](Window* win, int button, int action, int mods)
    {
        if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS || !cursorLocked)
            return;

        RaycastResult res = raycast(cam.position, cam.lookDir, config::REACH_DISTANCE, chunkManager);
        if (!res.hit)
            return;

        const auto positionInChunk = worldPosToChunkBlockPos(res.pos);
        if (comboSelection[comboIndex] == "None")
            res.chunk->setBlockUnsafe(positionInChunk, BLOCK_TYPE::AIR);
        else
        {
            BLOCK_TYPE blockType = BLOCK_TYPE::INVALID;
            if (comboSelection[comboIndex] == "Stone")
                blockType = BLOCK_TYPE::STONE;
            else if (comboSelection[comboIndex] == "Grass")
                blockType = BLOCK_TYPE::GRASS;
            else if (comboSelection[comboIndex] == "Sand")
                blockType = BLOCK_TYPE::SAND;
            else if (comboSelection[comboIndex] == "Wood")
                blockType = BLOCK_TYPE::WOOD;
            assert(blockType != BLOCK_TYPE::INVALID);

            glm::ivec3 offset;
            switch (res.face)
            {
                case BACK: offset = {0,0,-1}; break;
                case FRONT: offset = {0,0,1}; break;
                case LEFT: offset = {-1,0,0}; break;
                case RIGHT: offset = {1,0,0}; break;
                case TOP: offset = {0, 1, 0}; break;
                case BOTTOM: offset = {0, -1, 0}; break;
                default: assert(false);
            }
            glm::ivec3 neighbourBlockPos = positionInChunk + offset;

            if (inBounds(neighbourBlockPos))
                res.chunk->setBlockUnsafe(neighbourBlockPos, blockType);
            else
            {
                glm::uvec3 blockPosInOtherChunk = neighbourBlockPos;
                if (neighbourBlockPos.x == Chunk::CHUNK_SIZE) blockPosInOtherChunk.x = 0;
                else if (neighbourBlockPos.x == -1) blockPosInOtherChunk.x = Chunk::CHUNK_SIZE - 1;
                if (neighbourBlockPos.z == Chunk::CHUNK_SIZE) blockPosInOtherChunk.z = 0;
                else if (neighbourBlockPos.z == -1) blockPosInOtherChunk.z = Chunk::CHUNK_SIZE - 1;

                Chunk* neighbourChunk = chunkManager.getChunk(res.chunk->chunkPosition + glm::ivec2{offset.x, offset.z});

                assert(neighbourChunk != nullptr);
                assert(neighbourChunk->getBlockSafe(blockPosInOtherChunk) != BLOCK_TYPE::INVALID);

                neighbourChunk->setBlockUnsafe(blockPosInOtherChunk, blockType);
            }
        }

        if (positionInChunk.x == 0)
        {
            Chunk* chunk = chunkManager.getChunk({res.chunk->chunkPosition.x - 1, res.chunk->chunkPosition.y});
            if (chunk) chunk->isMeshBaked = false;
        }
        else if (positionInChunk.x == Chunk::CHUNK_SIZE - 1)
        {
            Chunk* chunk = chunkManager.getChunk({res.chunk->chunkPosition.x + 1, res.chunk->chunkPosition.y});
            if (chunk) chunk->isMeshBaked = false;
        }
        if (positionInChunk.z == 0)
        {
            Chunk* chunk = chunkManager.getChunk({res.chunk->chunkPosition.x, res.chunk->chunkPosition.y - 1});
            if (chunk) chunk->isMeshBaked = false;
        }
        else if (positionInChunk.z == Chunk::CHUNK_SIZE - 1)
        {
            Chunk* chunk = chunkManager.getChunk({res.chunk->chunkPosition.x, res.chunk->chunkPosition.y + 1});
            if (chunk) chunk->isMeshBaked = false;
        }
    });

    window.setCursorDisabled(cursorLocked);
#pragma endregion

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
    ImGui_ImplGlfw_InitForOpenGL(window.getGLFWWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    GLuint basicShader = createShader("../resources/shaders/BasicVert.glsl", "../resources/shaders/BasicFrag.glsl");
    GLuint blockShader = createShader("../resources/shaders/BlockVert.glsl", "../resources/shaders/BlockFrag.glsl");
    Texture textureAtlas("../resources/textures/TextureAtlas.png");
    auto axisVbo = createAxesVAO();

    float exposure = 1;
    float lastTime = glfwGetTime();

    std::deque<float> frameTimes;
    float frameTimeAccumulator = 0.0f;
    const float frameTimeWindow = 5.0f;

    while (window.isRunning())
    {
        if (debugMode)
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

        float skyExposure = 0.5f + 0.5f * exposure;
        glClearColor(0.5f * skyExposure, 0.8f * skyExposure, 0.9f * skyExposure, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const float currentTime = glfwGetTime();
        const float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        frameTimes.push_back(deltaTime);
        frameTimeAccumulator += deltaTime;
        while (frameTimeAccumulator > frameTimeWindow)
        {
            frameTimeAccumulator -= frameTimes.front();
            frameTimes.pop_front();
        }

        const auto vel = rawInput(window);
        if (glm::length(vel) > 0.0f)
            cam.move(glm::normalize(vel) * deltaTime * camSpeed);
        cam.updateView();

#pragma region draw
        textureAtlas.bind(0);
        bind(blockShader);
        setUniformMat4(blockShader, "u_VP", cam.viewProjection);
        setUniform1i(blockShader, "u_textureSlot", 0);
        setUniform3f(blockShader, "u_exposure", glm::vec3{exposure});

        auto chunkPos = worldPosToChunkPos(cam.position);
        chunkManager.unloadChunks(chunkPos);
        chunkManager.loadChunks(chunkPos);
        chunkManager.drawChunks(blockShader, chunkPos);

        RaycastResult res = raycast(cam.position, cam.lookDir, config::REACH_DISTANCE, chunkManager);
        if (res.hit)
            drawHighlightBlock(res.pos, res.chunk->chunkPosition, blockShader);

        if (debugMode)
        {
            axisVbo.bind();
            bind(basicShader);
            setUniformMat4(basicShader, "u_VP", cam.viewProjection);
            setUniform3f(basicShader, "u_GlobalPosition", cam.position + cam.lookDir);
            GLCall(glDrawArrays(GL_LINES, 0, axisVbo.vertexCount));

            ImGui::Begin("Debug");

            ImGui::Text("Frame data for last %.1f seconds:", frameTimeWindow);
            const float sum = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0f);
            const float avgFrameTime = sum / frameTimes.size();
            ImGui::Text("Avg frame time: %.3f ms (%.1f FPS)", avgFrameTime * 1000.0f, 1.0f / avgFrameTime);
            float maxFrameTime = *std::ranges::max_element(frameTimes.begin(), frameTimes.end());
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

#pragma endregion

        glfwSwapBuffers(window.getGLFWWindow());
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();

    PROFILER_END();
    return 0;
}

void drawHighlightBlock(const glm::vec3& worldPos, const glm::uvec2& chunkPos, const GLuint shader)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glm::uvec3 positionInChunk = worldPosToChunkBlockPos(worldPos);
    uint32_t buffer[36];
    uint32_t index = 0;
    for (uint32_t i = 0; i < 6; i++)
    {
        const glm::uvec2 atlasOffset = getAtlasOffset(BLOCK_TYPE::HIGHLIGHTED, FACE(0));
        const blockdata packedData = ((i & 0xF) << 28) |
              ((positionInChunk.x & 0x1F) << 23) |
              ((positionInChunk.y & 0x1F) << 18) |
              ((positionInChunk.z & 0x1F) << 13) |
              ((atlasOffset.x & 0xF) << 9) |
              ((atlasOffset.y & 0xF) << 5);
        for (uint32_t j = 0; j < 6; j++)
            buffer[index++] = packedData;
    }
    VertexArray highlightVao;
    VertexBufferLayout highlightLayout;
    highlightLayout.pushUInt(1);
    highlightVao.addBuffer(createBuffer(buffer, sizeof(buffer)), highlightLayout);
    highlightVao.vertexCount = 36;
    highlightVao.bind();
    setUniform3f(shader, "u_chunkOffset", {chunkPos.x * Chunk::CHUNK_SIZE, 0, chunkPos.y * Chunk::CHUNK_SIZE});
    glDrawArrays(GL_TRIANGLES, 0, highlightVao.vertexCount);
    glDepthFunc(GL_LESS);
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

glm::vec3 rawInput(const Window& window)
{
    glm::vec3 input(0.0f);
    input.z +=  1.0f * window.isKeyDown(GLFW_KEY_W);
    input.z += -1.0f * window.isKeyDown(GLFW_KEY_S);
    input.x += -1.0f * window.isKeyDown(GLFW_KEY_A);
    input.x +=  1.0f * window.isKeyDown(GLFW_KEY_D);
    input.y +=  1.0f * window.isKeyDown(GLFW_KEY_SPACE);
    input.y += -1.0f * window.isKeyDown(GLFW_KEY_LEFT_SHIFT);

    return input;
}