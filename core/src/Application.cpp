#include "Application.h"
#include <ranges>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "cstmlib/Log.h"
#include "GLFW/glfw3.h"

static void glfwErrorCallback(int error_code, const char* description)
{
    LOG_ERROR("GLFW Error ({}): {}", error_code, description);
    exit(1);
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

    m_Window = Window(settings);
    m_Window.bind();

#pragma region callbacks
    glfwSetCursorPosCallback(m_Window.getHandle(), [](GLFWwindow* window, double xpos, double ypos)
    {
        Event e(EventType::CursorMoved, core::Application::get().getWindow());
        e.cursorEvent.pos = {xpos, ypos};
        core::Application::get().propagateEvent(e);
    });
    glfwSetMouseButtonCallback(m_Window.getHandle(), [](GLFWwindow* window, int button, int action, int mods)
    {
        const EventType type = action == GLFW_PRESS ? EventType::MouseButtonPressed : EventType::MouseButtonReleased;
        Event e(type, core::Application::get().getWindow());
        e.mouseEvent.button = button;
        core::Application::get().propagateEvent(e);
    });
    glfwSetKeyCallback(m_Window.getHandle(), [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        const EventType type = action == GLFW_PRESS ? EventType::KeyPressed : EventType::KeyReleased;
        Event e(type, core::Application::get().getWindow());
        e.keyEvent.key = key;
        core::Application::get().propagateEvent(e);
    });
    glfwSetWindowCloseCallback(m_Window.getHandle(), [](GLFWwindow* window)
    {
        Event e(EventType::WindowClose, core::Application::get().getWindow());
        core::Application::get().propagateEvent(e);
    });
#pragma endregion

    // load opengl
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        LOG_ERROR("Could not load glad");
        exit(1);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_Window.getHandle(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

core::Application::~Application()
{
    for (const auto & layer : std::ranges::reverse_view(m_Layers))
        layer->onDetach();
    m_Layers.clear();
    m_Window.destroy();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}

core::Application& core::Application::get()
{
    assert(s_Instance);
    return *s_Instance;
}

void core::Application::suspendLayer(const std::string& name) const
{
    for (const auto& l : m_Layers)
    {
        if (l->m_Name == name)
        {
            LOG_INFO("suspending layer: {}", name);
            l->m_Enabled = false;
            return;
        }
    }
}

void core::Application::resumeLayer(const std::string& name) const
{
    for (const auto& l : m_Layers)
    {
        if (l->m_Name == name)
        {
            LOG_INFO("resuming layer: {}", name);
            l->m_Enabled = true;
            return;
        }
    }
}

void core::Application::removeLayer(const std::string& name)
{
    for (auto it = m_Layers.begin(); it != m_Layers.end(); ++it)
    {
        if ((*it)->m_Name == name)
        {
            LOG_INFO("Removing layer: {}", name);
            (*it)->onDetach();
            m_Layers.erase(it);
            return;
        }
    }
}

core::Layer* core::Application::getLayer(const std::string& name)
{
    for (const auto& l : m_Layers)
    {
        if (l->m_Name == name && l->m_Enabled)
            return l.get();
    }

    return nullptr;
}

void core::Application::run()
{
    m_Running = true;
    double lastTime = getTime();
    while (m_Running)
    {
        if (m_Window.shouldClose())
        {
            stop();
            break;
        }

        const double currentTime = getTime();
        const double deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        for (const auto& l : m_Layers)
            if (l->m_Enabled) l->onUpdate(deltaTime);

        for (const auto& l : m_Layers)
            if (l->m_Enabled) l->onRender();

        glfwSwapBuffers(m_Window.getHandle());
        glfwPollEvents();
    }
}

void core::Application::stop()
{
    m_Running = false;
}

void core::Application::propagateEvent(Event& e) const
{
    for (const auto& l : std::ranges::reverse_view(m_Layers))
        if (l->m_Enabled)
            if (l->onEvent(e)) break;
}

double core::Application::getTime() const
{
    using namespace std::chrono;
    return duration<double>(high_resolution_clock::now().time_since_epoch()).count();
}
