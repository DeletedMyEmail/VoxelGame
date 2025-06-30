#define GLM_ENABLE_EXPERIMENTAL
#define GLT_IMPLEMENTATION
#define GLT_MANUAL_VIEWPORT
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glText/gltext.h>
#include "glm/gtx/dual_quaternion.hpp"
#include <cstmlib/Profiling.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Camera.h"
#include "Chunk.h"
#include "FastNoiseLite.h"
#include "Shader.h"
#include "Window.h"
#include "OpenGLHelper.h"
#include "Raycast.h"
#include "Texture.h"
#include "VertexArray.h"

VertexArray createAxesVAO();
glm::vec3 rawInput(const Window& window, const glm::vec3& dir);
void drawText(const std::string& txt);

int main(int argc, char* argv[])
{
    LOG_INIT();
    PROFILER_INIT(0);

    bool debugMode = true;
    bool cursorLocked = true;

    Window window;
    Camera cam(glm::vec3{0,0,-10}, 60.0f, window.getWidth(), window.getHeight(), 0.1f, 1000.0f);

    std::vector<Chunk> chunks;
    BIOME b = FOREST;
    uint32_t worldSize = 30;
    FastNoiseLite noise = createBiomeNoise(b, 1337);
    for (uint32_t x = 0; x < worldSize; x++)
        for (uint32_t z = 0; z < worldSize; z++)
            chunks.emplace_back(glm::uvec2{x, z}, noise, b);

#pragma region window
    glm::dvec2 prevCursorPos = window.getMousePosition();

    window.onKey([&chunks, &debugMode, &cursorLocked] (Window* win, const int key, const int scancode, const int action, const int mods)
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
            for (auto& chunk : chunks)
                chunk.isDirty = true;

    });
    window.onCursorMove([&cam, &prevCursorPos, &cursorLocked](Window* win, const glm::dvec2 pos)
        {
            const glm::dvec2 offset = pos - prevCursorPos;
            if (cursorLocked)
                cam.rotate({offset.x, offset.y});
            prevCursorPos = pos;
        });

    float camSpeed = 70.0f;
    window.onScroll([&camSpeed](Window* win, glm::vec2 offset)
        {
            camSpeed += offset.y * 2.0f;
            if (camSpeed < 1.f)
                camSpeed = 1.f;
        });
    window.onMouseButton([&cam, &chunks, worldSize, &cursorLocked](Window* win, int button, int action, int mods)
    {
        if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS || !cursorLocked)
            return;

        RaycastResult res = raycast(cam.position, cam.lookDir, 15.0f, glm::ivec3{worldSize * Chunk::CHUNK_SIZE, Chunk::MAX_HEIGHT, worldSize * Chunk::CHUNK_SIZE}, chunks);
        if (res.hit)
        {
            const auto positionInChunk = worldPosToChunkBlockPos(res.pos);
            res.chunk->setBlockUnsafe(positionInChunk, BLOCK_TYPE::AIR);
            if (positionInChunk.x == 0)
                getChunk(chunks, res.chunk->chunkPosition);

            Chunk* c = nullptr;
            if (positionInChunk.x == Chunk::CHUNK_SIZE - 1)
                c = getChunk(chunks, res.chunk->chunkPosition + glm::uvec2{1, 0});
            else if (positionInChunk.x == 0)
                c = getChunk(chunks, res.chunk->chunkPosition + glm::uvec2{-1, 0});
            if (c != nullptr)
                c->isDirty = true;
            if (positionInChunk.z == Chunk::CHUNK_SIZE - 1)
                c = getChunk(chunks, res.chunk->chunkPosition + glm::uvec2{0, 1});
            else if (positionInChunk.z == 0)
                c = getChunk(chunks, res.chunk->chunkPosition + glm::uvec2{0, -1});
            if (c != nullptr)
                c->isDirty = true;

        }
    });

    window.setCursorDisabled(cursorLocked);
#pragma endregion

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

        const auto vel = rawInput(window, cam.lookDir);
        if (glm::length(vel) > 0.0f)
            cam.move(glm::normalize(vel) * deltaTime * camSpeed);
        cam.updateView();

        textureAtlas.bind(0);
        bind(blockShader);
        setUniformMat4(blockShader, "u_VP", cam.viewProjection);
        setUniform1i(blockShader, "u_textureSlot", 0);
        setUniform3f(blockShader, "u_exposure", glm::vec3{exposure});

        uint32_t chunksBaked = 0;
        uint32_t MAX_BAKE_PER_FRAME = 4;
        for (auto& chunk : chunks)
        {
            if (chunk.isDirty)
            {
                if (chunksBaked >= MAX_BAKE_PER_FRAME)
                    continue;

                Chunk* neighbors[3][3];
                for (int dx = -1; dx <= 1; dx++)
                {
                    for (int dz = -1; dz <= 1; dz++)
                    {
                        if (dx == 0 && dz == 0) // skip self
                            continue;

                        glm::uvec2 neighbourPos = glm::uvec2{dx, dz} + chunk.chunkPosition;
                        Chunk* neighbour = getChunk(chunks, neighbourPos);

                        if (!neighbour)
                            neighbors[dx + 1][dz + 1] = nullptr;
                        else
                            neighbors[dx + 1][dz + 1] = &(*neighbour);
                    }
                }

                chunk.bake(neighbors);
                chunksBaked++;
            }

            chunk.vao.bind();
            setUniform3f(blockShader, "u_chunkOffset", {chunk.chunkPosition.x * Chunk::CHUNK_SIZE, 0, chunk.chunkPosition.y * Chunk::CHUNK_SIZE});
            GLCall(glDrawArrays(GL_TRIANGLES, 0, chunk.vao.vertexCount));
        }

        RaycastResult res = raycast(cam.position, cam.lookDir, 15.0f, glm::ivec3{worldSize * Chunk::CHUNK_SIZE, Chunk::MAX_HEIGHT, worldSize * Chunk::CHUNK_SIZE}, chunks);
        if (res.hit)
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            glm::uvec3 positionInChunk = worldPosToChunkBlockPos(res.pos);
            uint32_t buffer[36];
            uint32_t index = 0;
            for (uint32_t i = 0; i < 6; i++)
            {
                const glm::uvec2 atlasOffset = getAtlasOffset(BLOCK_TYPE::HIGHLIGHTED, 0);
                blockdata packedData = (i << 28) | (positionInChunk.x << 24) | (positionInChunk.y << 16) | (positionInChunk.z << 12) | (atlasOffset.x << 8) | (atlasOffset.y << 4);
                for (uint32_t j = 0; j < 6; j++)
                {
                    buffer[index++] = packedData;
                }
            }
            VertexArray highlightVao;
            VertexBufferLayout highlightLayout;
            highlightLayout.pushUInt(1);
            highlightVao.addBuffer(createBuffer(buffer, sizeof(buffer)), highlightLayout);
            highlightVao.vertexCount = 36;
            highlightVao.bind();
            setUniform3f(blockShader, "u_chunkOffset", {res.chunk->chunkPosition.x * Chunk::CHUNK_SIZE, 0, res.chunk->chunkPosition.y * Chunk::CHUNK_SIZE});
            glDrawArrays(GL_TRIANGLES, 0, highlightVao.vertexCount);
            glDepthFunc(GL_LESS);
        }

        if (debugMode)
        {
            axisVbo.bind();
            bind(basicShader);
            setUniformMat4(basicShader, "u_VP", cam.viewProjection);
            setUniform3f(basicShader, "u_GlobalPosition", cam.position + cam.lookDir);
            GLCall(glDrawArrays(GL_LINES, 0, axisVbo.vertexCount));

            ImGui::Begin("Debug");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::SliderFloat("Exposure", &exposure, 0.0f, 1.0f);
            ImGui::SliderFloat("Camera Speed", &camSpeed, 1.0f, 200.0f);
            ImGui::End();
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

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

VertexArray createAxesVAO()
{
    float axisVertices[] =
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

glm::vec3 rawInput(const Window& window, const glm::vec3& dir)
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
