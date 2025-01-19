#include "App.h"
#include <Log.h>
#include "Metrics.h"

App::App()
    :   m_Window(1920, 1080),
        m_GameWorld(glm::vec3{2,Chunk::CHUNK_SIZE+2,2}, 4),
        m_Camera(glm::vec3{4,Chunk::CHUNK_SIZE+3,4}, 90.0f, m_Window.getSettings().height, m_Window.getSettings().width, 0.1f, 1000.0f)
{
    m_Window.setCursorDisabled(true);
    m_Window.onScroll([this](Window* win, glm::dvec2 offset)
         {
             m_GameWorld.getPlayer().increaseSpeed(offset.y);
         });
    m_Window.onKey([this](Window* win, const int key, const int scancode, const int action, const int mods)
         {
            if (action == GLFW_PRESS)
                processHotkeys(key, scancode, mods);
         });
}

void App::run()
{
    const Shader debugShader("../shader/DebugVert.glsl", "../shader/DebugFrag.glsl", nullptr);
    FrameMetrics metrics;

    while (m_Window.isRunning())
    {
        metrics.update();
        if (metrics.getDeltaSum() >= 1.0f)
        {
            m_Window.setTitle(metrics);
            metrics.reset();
        }

        m_Renderer.clear(m_Window);

        m_GameWorld.update(metrics.getDeltaTime(), getPlayerMoveInputs(), m_Camera.getLookDir());

        glm::vec3 playerPos = m_GameWorld.getPlayer().getPhysics().getPosition();
        glm::vec3 dif = playerPos - m_Camera.getPosition();
        if (dif != glm::vec3(0))
            m_Camera.translate(dif);
        m_Camera.update(m_Window.getMousePosition());

        for (Chunk& chunk : m_GameWorld.getChunks())
            m_Renderer.drawChunk(chunk, m_Window, m_Camera);

        //drawPlayer(player.getPhysics().getPosition(), m_Window, m_Camera);
        //drawAxes(m_Window, m_Camera);

    }
}

void App::processHotkeys(const int key, int scancode, int mods)
{
    if (key == GLFW_KEY_ESCAPE)
        exit(0);
    if (key == GLFW_KEY_TAB)
    {
        GLint mode;
        GLCall(glGetIntegerv(GL_POLYGON_MODE, &mode))
        GLCall(glPolygonMode( GL_FRONT_AND_BACK, mode == GL_FILL ? GL_LINE : GL_FILL );)
    }
}

glm::vec3 App::getPlayerMoveInputs() const
{
    glm::vec3 movement(0.0f);
    if (m_Window.isKeyDown(GLFW_KEY_W)) movement.z += 1.0f;
    if (m_Window.isKeyDown(GLFW_KEY_S)) movement.z -= 1.0f;
    if (m_Window.isKeyDown(GLFW_KEY_A)) movement.x += 1.0f;
    if (m_Window.isKeyDown(GLFW_KEY_D)) movement.x -= 1.0f;
    if (m_Window.isKeyDown(GLFW_KEY_SPACE)) movement.y += 1.0f;
    if (m_Window.isKeyDown(GLFW_KEY_LEFT_SHIFT)) movement.y -= 1.0f;

    return movement;
}
