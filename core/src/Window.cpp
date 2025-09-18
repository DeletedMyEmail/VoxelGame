#include "Window.h"
#include "cstmlib/Log.h"
#include "OpenGLHelper.h"

Window::Window(WindowSettings& settings)
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    if (settings.fullscreen)
    {
        m_Handle = glfwCreateWindow(mode->width, mode->height, settings.title, glfwGetPrimaryMonitor(), nullptr);
        settings.width = mode->width;
        settings.height = mode->height;
    }
    else
        m_Handle = glfwCreateWindow(settings.width, settings.height, settings.title, nullptr, nullptr);

    bind();
    glfwSwapInterval(settings.vSync);
}

Window::~Window()
{
    destroy();
}

Window& Window::operator=(Window&& other)
{
    if (this == &other)
        return *this;

    m_Handle = other.m_Handle;
    other.m_Handle = nullptr;

    return *this;
}

void Window::destroy()
{
    LOG_INFO("Destroying window {}", (long) m_Handle);
    if (m_Handle)
    {
        glfwDestroyWindow(m_Handle);
        m_Handle = nullptr;
    }
}

void Window::disableCursor(const bool disable) const
{
    glfwSetInputMode(m_Handle, GLFW_CURSOR, disable ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(m_Handle);
}

glm::dvec2 Window::getMousePosition() const
{
    double x, y;
    glfwGetCursorPos(m_Handle, &x, &y);
    return {x, y};
}

bool Window::isKeyDown(const int key) const
{
    const int state = glfwGetKey(m_Handle, key);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Window::isMouseButtonDown(const int button) const
{
    const int state = glfwGetMouseButton(m_Handle, button);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

void Window::bind() const
{
    glfwMakeContextCurrent(m_Handle);
}