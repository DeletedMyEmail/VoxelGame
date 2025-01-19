#include "Renderer.h"
#include "App.h"

#include <Log.h>

#include "Metrics.h"

App::App()
    :   m_Window(WindowBuilder().
                 size(1200, 1900).
                 culling().
                 disableCursor().
                 onScroll([this](Window* win, const double x, const double y)
                 {
                     this->m_GameWorld.getPlayer().increaseSpeed(y);
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
        m_GameWorld(glm::vec3{2,Chunk::CHUNK_SIZE+2,2}, 4),
        m_TextureAtlas("../resources/textureAtlas.png"),
        m_Shader("../shader/DefaultVert.glsl", "../shader/DefaultFrag.glsl", nullptr),
        m_Camera(glm::vec3{4,Chunk::CHUNK_SIZE+3,4}, 90.0f, m_Window.getSettings().height, m_Window.getSettings().width, 0.1f, 1000.0f)
{
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

        clear(m_Window);

        m_GameWorld.update(metrics.getDeltaTime(), m_Window, m_Camera.getLookDir());

        glm::vec3 playerPos = m_GameWorld.getPlayer().getPhysics().getPosition();
        glm::vec3 dif = playerPos - m_Camera.getPosition();
        if (dif != glm::vec3(0))
            m_Camera.translate(dif);
        m_Camera.update(m_Window.getMousePosition());

        for (Chunk& chunk : m_GameWorld.getChunks())
            drawChunk(chunk, m_Shader, m_TextureAtlas, m_Window, m_Camera);

        //drawPlayer(player.getPhysics().getPosition(), m_Window, debugShader, m_Camera);
        //drawAxes(m_Window, debugShader, m_Camera);

    }
}