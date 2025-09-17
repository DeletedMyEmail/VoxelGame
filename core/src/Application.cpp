#include "Application.h"
#include "../../app/include/Metrics.h"
#include "cstmlib/Log.h"
#include "GLFW/glfw3.h"

static void glfwErrorCallback(int error_code, const char* description)
{
    LOG_ERROR("GLFW Error ({}): {}", error_code, description);
    //exit(1);
}

static core::Application* s_Instance = nullptr;

core::Application::Application(WindowSettings& settings)
{
    s_Instance = this;

    if (!glfwInit())
    {
        LOG_ERROR("Could not initialize GLFW");
        exit(1);
    }
    glfwSetErrorCallback(glfwErrorCallback);
    //glfwSetTime(std::chrono::system_clock::now().time_since_epoch().count() / 1000.0);

    m_Window = Window(settings);
    m_Window.bind();

    glfwMakeContextCurrent(m_Window.getHandle());
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        LOG_ERROR("Could not load glad");
        exit(1);
    }
}

core::Application::~Application()
{
    for (Layer& l : m_Layers)
        l.onDetach();

    m_Window.destroy();
    glfwTerminate();
}

core::Application& core::Application::get()
{
    assert(s_Instance);
    return *s_Instance;
}

void core::Application::run()
{
    m_Running = true;
    float lastTime = getTime();
    while (m_Running)
    {
        if (m_Window.shouldClose())
        {
            stop();
            break;
        }

        const float currentTime = getTime();
        const float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // event system...

        for (Layer& l : m_Layers)
            l.onUpdate(deltaTime);

        for (Layer& l : m_Layers)
            l.onRender();

        glfwSwapBuffers(m_Window.getHandle());
        glfwPollEvents();
    }
}

void core::Application::stop()
{
    m_Running = false;
}

double core::Application::getTime() const
{
    return glfwGetTime();
}
