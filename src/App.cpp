#include "App.h"
#include "Chunk.h"
#include "Noise.h"
#include "Renderer.h"
#include "Metrics.h"
#include "stb/stb_image_write.h"

App::App()
    :   m_Window(WindowBuilder().
                 size(1200, 1900).
                 title("Minecraft Clone").
                 disableCursor().
                 culling().
                 onScroll([this](Window* win, const double x, const double y)
                 {
                     this->m_Camera.increaseSpeed(y);
                 }).
                 onClose([](Window*)
                 {
                     exit(0);
                 }).
                 onKey([](Window* win, const int key, const int scancode, const int action, int mods)
                 {
                     if (key == GLFW_KEY_ESCAPE)
                         exit(0);
                     if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
                     {
                         GLint mode;
                         GLCall(glGetIntegerv(GL_POLYGON_MODE, &mode))
                         GLCall(glPolygonMode( GL_FRONT_AND_BACK, mode == GL_FILL ? GL_LINE : GL_FILL );)
                     }
                 }).
                 build()),
        m_TextureAtlas("../resources/textureAtlas.png"),
        m_Shader("../shader/BlockVert.glsl", "../shader/BlockFrag.glsl", nullptr),
        m_Camera(glm::vec3{0,0,-2}, 90.0f, m_Window.getSettings().height, m_Window.getSettings().width, 0.1f, 1000.0f)
{
    initChunks(4);
}

void App::run()
{
    auto prevMousePos = m_Window.getMousePosition();
    FrameMetrics metrics;
    while (m_Window.isRunning())
    {
        metrics.update();
        if (metrics.getDeltaSum() >= 1.0f)
        {
            m_Window.setTitle(metrics);
            metrics.reset();
        }

        clear(m_Window);

        processCamInputs(prevMousePos, metrics.getDeltaTime());
        m_Camera.update();

        for (Chunk& chunk : m_Chunks)
            draw(chunk, m_Shader, m_TextureAtlas, m_Window, m_Camera);
    }
}

static glm::vec3 getCamMovement(const Window& window)
{
    glm::vec3 movement(0.0f);
    if (window.isKeyDown(GLFW_KEY_W)) movement.z += 1.0f;
    if (window.isKeyDown(GLFW_KEY_S)) movement.z -= 1.0f;
    if (window.isKeyDown(GLFW_KEY_A)) movement.x -= 1.0f;
    if (window.isKeyDown(GLFW_KEY_D)) movement.x += 1.0f;
    if (window.isKeyDown(GLFW_KEY_SPACE)) movement.y += 1.0f;
    if (window.isKeyDown(GLFW_KEY_LEFT_SHIFT)) movement.y -= 1.0f;

    return movement;
}

void App::processCamInputs(glm::dvec2& prevMousePos, const float deltaTime)
{
    const auto mousePos = m_Window.getMousePosition();
    if (mousePos != prevMousePos)
    {
        const auto relMouseMovement = mousePos - prevMousePos;

        m_Camera.rotate(relMouseMovement.x, relMouseMovement.y);
        prevMousePos = mousePos;
    }

    auto movement = getCamMovement(m_Window);
    if (movement != glm::vec3(0))
    {
        movement = glm::normalize(movement) * m_Camera.getSpeed() * deltaTime;
        m_Camera.move(movement);
    }
}

void App::initChunks(const unsigned int chunksPerSide)
{
    const unsigned int size = Chunk::CHUNK_SIZE * chunksPerSide;
    unsigned char** heightMap = genPerlinMap(size, size, Chunk::MAX_HEIGHT / 2, Chunk::MAX_HEIGHT, 42);

    m_Chunks.reserve(chunksPerSide * chunksPerSide);
    for (unsigned int x = 0; x < chunksPerSide; x++)
    {
        for (unsigned int z = 0; z < chunksPerSide; z++)
        {
            m_Chunks.emplace_back(glm::uvec2{x, z}, heightMap);
        }
    }

    freeMap(heightMap, size);
}
