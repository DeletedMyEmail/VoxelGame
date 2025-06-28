#define GLM_ENABLE_EXPERIMENTAL
#define GLT_IMPLEMENTATION
#define GLT_MANUAL_VIEWPORT
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glText/gltext.h>
#include "glm/gtx/dual_quaternion.hpp"
#include <cstmlib/Profiling.h>

#include "Camera.h"
#include "Chunk.h"
#include "FastNoiseLite.h"
#include "Shader.h"
#include "Window.h"
#include "OpenGLHelper.h"
#include "Texture.h"
#include "VertexArray.h"

VertexArray createAxesVAO();
glm::vec3 moveInput(const Window& window, const glm::vec3& dir);
void drawText(const std::string& txt);

int main(int argc, char* argv[])
{
    LOG_INIT();
    PROFILER_INIT(0);

#pragma region window
    Window window;
    Camera cam(glm::vec3{0,0,-10}, 90.0f, window.getWidth(), window.getHeight(), 0.1f, 1000.0f);
    glm::dvec2 prevCursorPos = window.getMousePosition();
    window.onKey([](Window* win, const int key, const int scancode, const int action, const int mods)
    {
        if (action != GLFW_PRESS)
            return;
        if (key == GLFW_KEY_ESCAPE)
            win->stop();
        else if (key == GLFW_KEY_V)
            win->setCursorDisabled(true);
        else if (key == GLFW_KEY_B)
            win->setCursorDisabled(false);
    });
    window.onCursorMove([&cam, &prevCursorPos](Window* win, const glm::dvec2 pos)
        {
            const glm::dvec2 offset = pos - prevCursorPos;
            cam.rotate({offset.x, offset.y});
            prevCursorPos = pos;
        });

#pragma endregion

    glEnable(GL_DEPTH_TEST);
    gltInit();
    gltViewport(window.getWidth(), window.getHeight());

    GLuint basicShader = createShader("../resources/shaders/BasicVert.glsl", "../resources/shaders/BasicFrag.glsl");
    GLuint blockShader = createShader("../resources/shaders/BlockVert.glsl", "../resources/shaders/BlockFrag.glsl");
    Texture textureAtlas("../resources/textures/TextureAtlas.png");

    unsigned frameCount = 0;
    float timeSinceDisplay = 0.0f;
    float lastTime = glfwGetTime();
    std::string text;

    auto axisVbo = createAxesVAO();

    BIOME b = HILLS;
    uint32_t worldSize = 30;
    FastNoiseLite noise = createBiomeNoise(b, 1337);

    std::vector<Chunk> chunks;
    for (uint32_t x = 0; x < worldSize; x++)
        for (uint32_t z = 0; z < worldSize; z++)
            chunks.emplace_back(glm::uvec2{x, z}, noise, b);

    while (window.isRunning())
    {
        glClearColor(0.5f, 0.8f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const float currentTime = glfwGetTime();
        const float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        timeSinceDisplay += deltaTime;
        frameCount++;

        const auto vel = moveInput(window, cam.lookDir);
        if (glm::length(vel) > 0.0f)
            cam.move(glm::normalize(vel) * deltaTime * 60.0f);
        cam.updateView();

        textureAtlas.bind(0);
        bind(blockShader);
        setUniformMat4(blockShader, "u_VP", cam.viewProjection);
        setUniform1i(blockShader, "u_textureSlot", 0);

        for (auto& chunk : chunks)
        {
            if (chunk.isDirty)
            {
                Chunk* neighbors[3][3];
                for (int dx = -1; dx <= 1; dx++)
                {
                    for (int dz = -1; dz <= 1; dz++)
                    {
                        if (dx == 0 && dz == 0) // skip self
                            continue;

                        glm::uvec2 neighbourPos = glm::uvec2{dx, dz} + chunk.chunkPosition;
                        auto neighbour = std::ranges::find_if(chunks,
                            [neighbourPos](const Chunk& c) { return c.chunkPosition == neighbourPos; });

                        if (neighbour == chunks.end())
                            neighbors[dx + 1][dz + 1] = nullptr;
                        else
                            neighbors[dx + 1][dz + 1] = &(*neighbour);
                    }
                }

                chunk.bake(neighbors);
            }

            chunk.vao.bind();
            setUniform3f(blockShader, "u_chunkOffset", {chunk.chunkPosition.x * Chunk::CHUNK_SIZE, 0, chunk.chunkPosition.y * Chunk::CHUNK_SIZE});
            GLCall(glDrawArrays(GL_TRIANGLES, 0, chunk.vao.vertexCount));
        }

#pragma region drawDebug
        axisVbo.bind();
        bind(basicShader);
        setUniformMat4(basicShader, "u_VP", cam.viewProjection);
        setUniform3f(basicShader, "u_GlobalPosition", cam.position + cam.lookDir);
        GLCall(glDrawArrays(GL_LINES, 0, axisVbo.vertexCount));
#pragma endregion

#pragma region metrics
        if (timeSinceDisplay >= 1.0f)
        {
            text = fmt::format("x: {:.2f}, y: {:.2f}, z: {:.2f}, fps: {}, avg frame time {}ms",
                        cam.position.x, cam.position.y, cam.position.z, std::to_string(frameCount), std::to_string(timeSinceDisplay / frameCount * 1000));
            frameCount = timeSinceDisplay = 0;
        }
        drawText(text);
#pragma endregion

        glfwSwapBuffers(window.getGLFWWindow());
        glfwPollEvents();
    }

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

glm::vec3 moveInput(const Window& window, const glm::vec3& dir)
{
    glm::vec3 input(0.0f);
    input.z +=  1.0f * window.isKeyDown(GLFW_KEY_W);
    input.z += -1.0f * window.isKeyDown(GLFW_KEY_S);
    input.x += -1.0f * window.isKeyDown(GLFW_KEY_A);
    input.x +=  1.0f * window.isKeyDown(GLFW_KEY_D);
    input.y +=  1.0f * window.isKeyDown(GLFW_KEY_SPACE);
    input.y += -1.0f * window.isKeyDown(GLFW_KEY_LEFT_SHIFT);

    /*const glm::vec3 right = -glm::normalize(cross(dir, glm::vec3(0.0f, 1.0f, 0.0f)));
    const glm::vec3 forward = glm::normalize(glm::vec3{dir.x, 0.0f, dir.z});
    static glm::vec3 up(0.0f, 1.0f, 0.0f);*/
    //return  forward * input.z + right * input.x + up * input.y;
    return input;
}

void drawText(const std::string& txt)
{
    GLCall(glFrontFace(GL_CW));
    GLTtext* glText = gltCreateText();
    gltSetText(glText, txt.c_str());
    gltBeginDraw();
    gltColor(1.0f, 1.0f, 1.0f, 1.0f);
    gltDrawText2D(glText, 10.0f, 10.0f, 2.0f);
    gltEndDraw();
    gltDeleteText(glText);
    GLCall(glFrontFace(GL_CCW))
}