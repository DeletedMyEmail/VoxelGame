#include "App.h"
#include "Renderer.h"
#include "Metrics.h"

App::App()
    :   m_Window(WindowBuilder().
                 size(1200, 1900).
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
        m_GameWorld(glm::vec3{4,23,4}, 4),
        m_TextureAtlas("../resources/textureAtlas.png"),
        m_Shader("../shader/DefaultVert.glsl", "../shader/DefaultFrag.glsl", nullptr),
        m_Camera(glm::vec3{4,23,4}, 90.0f, m_Window.getSettings().height, m_Window.getSettings().width, 0.1f, 1000.0f)
{
}

glm::vec3 getCamMoveInput(const Window& win);
glm::vec3 getPlayerMoveInputs(const Window& win);

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

        Player& player = m_GameWorld.getPlayer();
        player.getPhysics().addVelocity(getPlayerMoveInputs(m_Window) * metrics.getDeltaTime() * player.getSpeed());
        m_GameWorld.update();

        m_Camera.move(getCamMoveInput(m_Window) * metrics.getDeltaTime() * m_Camera.getSpeed());
        m_Camera.update(m_Window.getMousePosition());

        for (Chunk& chunk : m_GameWorld.getChunks())
            drawChunk(chunk, m_Shader, m_TextureAtlas, m_Window, m_Camera);

        drawPlayer(player.getPhysics().getPosition(), m_Window, debugShader, m_Camera);
        drawAxes(m_Window, debugShader, m_Camera);
    }
}

glm::vec3 getCamMoveInput(const Window& win)
{
    glm::vec3 movement(0.0f);
    if (win.isKeyDown(GLFW_KEY_W)) movement.z += 1.0f;
    if (win.isKeyDown(GLFW_KEY_S)) movement.z -= 1.0f;
    if (win.isKeyDown(GLFW_KEY_A)) movement.x += 1.0f;
    if (win.isKeyDown(GLFW_KEY_D)) movement.x -= 1.0f;
    if (win.isKeyDown(GLFW_KEY_SPACE)) movement.y += 1.0f;
    if (win.isKeyDown(GLFW_KEY_LEFT_SHIFT)) movement.y -= 1.0f;

    return movement;
}

glm::vec3 getPlayerMoveInputs(const Window& win)
{
    glm::vec3 movement(0.0f);
    if (win.isKeyDown(GLFW_KEY_UP)) movement.z += 1.0f;
    if (win.isKeyDown(GLFW_KEY_DOWN)) movement.z -= 1.0f;
    if (win.isKeyDown(GLFW_KEY_LEFT)) movement.x += 1.0f;
    if (win.isKeyDown(GLFW_KEY_RIGHT)) movement.x -= 1.0f;
    if (win.isKeyDown(GLFW_KEY_X)) movement.y += 1.0f;
    if (win.isKeyDown(GLFW_KEY_C)) movement.y -= 1.0f;

    return movement;
}