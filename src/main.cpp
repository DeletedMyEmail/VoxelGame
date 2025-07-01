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
void drawChunks(std::vector<Chunk>& chunks, GLuint shader, uint32_t maxChunkBakes, const glm::vec3& cameraPosition, float maxDistance);
void drawHighlightBlock(const glm::vec3& worldPos, const glm::uvec2& chunkPos, GLuint shader);

int main(int argc, char* argv[])
{
    LOG_INIT();
    PROFILER_INIT(0);

    bool debugMode = true;
    bool cursorLocked = true;
    float RENDER_DISTANCE = Chunk::CHUNK_SIZE * 32;
    const uint32_t MAX_BAKES = 4;

    Window window;
    Camera cam(glm::vec3{0,0,-10}, 60.0f, window.getWidth(), window.getHeight(), 0.1f, RENDER_DISTANCE);
    float camSpeed = 70.0f;

    std::vector<Chunk> chunks;
    BIOME b = HILLS;
    uint32_t worldSize = 128;
    FastNoiseLite noise = createBiomeNoise(b, 1337);
    for (uint32_t x = 0; x < worldSize; x++)
        for (uint32_t z = 0; z < worldSize; z++)
            chunks.emplace_back(glm::uvec2{x, z}, noise, b);

    std::array<const char*, 5> comboSelection{ "None", "Stone", "Grass", "Sand", "Wood"};
    int32_t comboIndex = 0;

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

    window.onMouseButton([&cam, &chunks, worldSize, &cursorLocked, &comboSelection, &comboIndex](Window* win, int button, int action, int mods)
    {
        if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS || !cursorLocked)
            return;

        RaycastResult res = raycast(cam.position, cam.lookDir, 15.0f, glm::ivec3{worldSize * Chunk::CHUNK_SIZE, Chunk::MAX_HEIGHT, worldSize * Chunk::CHUNK_SIZE}, chunks);
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

            glm::uvec3 offset;
            switch (res.face)
            {
                case BACK: offset = {0,0,-1}; break;
                case FRONT: offset = {0,1,1}; break;
                case LEFT: offset = {-1,0,0}; break;
                case RIGHT: offset = {1,0,0}; break;
                case TOP: offset = {0, 1, 0}; break;
                case BOTTOM: offset = {0, -1, 0}; break;
                default: assert(false);
            }
            glm::uvec3 neighbourBlockPos = positionInChunk + offset;

            if (inBounds(neighbourBlockPos))
                res.chunk->setBlockUnsafe(neighbourBlockPos, blockType);
            else
            {
                glm::uvec3 blockPosInOtherChunk = neighbourBlockPos;
                if (neighbourBlockPos.x == Chunk::CHUNK_SIZE) blockPosInOtherChunk.x = 0;
                else if (neighbourBlockPos.x > Chunk::CHUNK_SIZE) blockPosInOtherChunk.x = Chunk::CHUNK_SIZE - 1;
                if (neighbourBlockPos.z == Chunk::CHUNK_SIZE) blockPosInOtherChunk.z = 0;
                else if (neighbourBlockPos.z > Chunk::CHUNK_SIZE) blockPosInOtherChunk.z = Chunk::CHUNK_SIZE - 1;

                Chunk* neighborChunk = getChunk(chunks, res.chunk->chunkPosition + glm::uvec2{offset.x, offset.z});
                if (!neighborChunk)
                    return;

                assert(neighborChunk->getBlockSafe(blockPosInOtherChunk) != BLOCK_TYPE::INVALID);
                neighborChunk->setBlockUnsafe(blockPosInOtherChunk, blockType);
            }
        }

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


#pragma region draw
        textureAtlas.bind(0);
        bind(blockShader);
        setUniformMat4(blockShader, "u_VP", cam.viewProjection);
        setUniform1i(blockShader, "u_textureSlot", 0);
        setUniform3f(blockShader, "u_exposure", glm::vec3{exposure});

        drawChunks(chunks, blockShader, MAX_BAKES, cam.position, RENDER_DISTANCE);

        RaycastResult res = raycast(cam.position, cam.lookDir, 15.0f, glm::ivec3{worldSize * Chunk::CHUNK_SIZE, Chunk::MAX_HEIGHT, worldSize * Chunk::CHUNK_SIZE}, chunks);
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
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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
    setUniform3f(shader, "u_chunkOffset", {chunkPos.x * Chunk::CHUNK_SIZE, 0, chunkPos.y * Chunk::CHUNK_SIZE});
    glDrawArrays(GL_TRIANGLES, 0, highlightVao.vertexCount);
    glDepthFunc(GL_LESS);
}

void drawChunks(std::vector<Chunk>& chunks, const GLuint shader, const uint32_t maxChunkBakes, const glm::vec3& cameraPosition, const float maxDistance)
{
    glm::vec2 camPosXZ{ cameraPosition.x, cameraPosition.z };

    uint32_t chunksBaked = 0;
    for (auto& chunk : chunks)
    {
        glm::vec2 chunkPosWorld = glm::vec2(chunk.chunkPosition) * (float) Chunk::CHUNK_SIZE;

        if (glm::distance(chunkPosWorld, camPosXZ) > maxDistance)
            continue;

        if (chunk.isDirty)
        {
            if (chunksBaked >= maxChunkBakes)
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
        setUniform3f(shader, "u_chunkOffset", {chunk.chunkPosition.x * Chunk::CHUNK_SIZE, 0, chunk.chunkPosition.y * Chunk::CHUNK_SIZE});

        GLCall(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, chunk.vao.vertexCount));
    }
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
