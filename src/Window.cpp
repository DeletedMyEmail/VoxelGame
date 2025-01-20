#include "Window.h"
#include "Log.h"
#include "Renderer.h"

bool Window::s_glfwInitialized = false;

Window::Window()
{
    m_Settings.fullscreen = true;
    init();
}

Window::Window(const int width, const int height)
{
    m_Settings.width = width;
    m_Settings.height = height;
    init();
}

Window::~Window()
{
    GLCall(glfwDestroyWindow(m_Window))
}

void Window::init()
{
    if (!s_glfwInitialized)
        glfwInit();

    createGLFWWindow();
    glfwSetWindowUserPointer(m_Window, this);

    glfwMakeContextCurrent(m_Window);
    gladLoadGL();
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        LOG_ERROR("Could not load glad");
    }

    setVSync(false);
}

void Window::setVSync(const bool enabled)
{
    m_Settings.vysnc = enabled;
    GLCall(glfwSwapInterval(enabled))
}

void Window::setCursorDisabled(const bool disabled)
{
    m_Settings.disableCursor = disabled;
    const int mode = disabled ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;

    GLCall(glfwSetInputMode(m_Window, GLFW_CURSOR, mode))
}

void Window::createGLFWWindow()
{
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    if (m_Settings.fullscreen)
    {
        m_Window = glfwCreateWindow(mode->width, mode->height, m_Settings.title, glfwGetPrimaryMonitor(), nullptr);
        m_Settings.width = mode->width;
        m_Settings.height = mode->height;
    }
    else
    {
        m_Window = glfwCreateWindow(m_Settings.width, m_Settings.height, m_Settings.title, nullptr, nullptr);
    }

    if (!m_Window)
    {
        LOG_ERROR("Could not create Window");
    }
}

bool Window::isRunning() const
{
    return !glfwWindowShouldClose(m_Window);
}

void Window::stop() const
{
    GLCall(glfwSetWindowShouldClose(m_Window, GL_TRUE))
}

glm::dvec2 Window::getMousePosition() const
{
    double x, y;
    GLCall(glfwGetCursorPos(m_Window, &x, &y))
    return {x, y};
}

double Window::getMouseX() const {
    return getMousePosition().x;
}

double Window::getMouseY() const {
    return getMousePosition().y;
}

bool Window::isKeyDown(const int pKey) const
{
    const int state = glfwGetKey(m_Window, pKey);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Window::isMouseButtonPressed(const int pButton) const
{
    const int state = glfwGetMouseButton(m_Window, pButton);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

void Window::setTitle(const std::string& title) const
{
    GLCall(glfwSetWindowTitle(m_Window, title.c_str()))
}

void Window::onCursorMove(const cursorCallback& callback)
{
    m_Settings.onCursorMove = callback;
    GLCall(glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, const double xpos, const double ypos)
    {
        auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        win->m_Settings.onCursorMove(win, glm::dvec2{xpos, ypos});
    }))
}

void Window::onClose(const closeCallback& callback)
{
    m_Settings.onClose = callback;
    GLCall(glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
    {
        auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        win->m_Settings.onClose(win);
    }))
}

void Window::onMouseButton(const mouseButtonCallback& callback)
{
    m_Settings.onMouseButton = callback;
    GLCall(glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, const int button, const int action, const int mods)
    {
        auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        win->m_Settings.onMouseButton(win, button, action, mods);
    }))
}


void Window::onKey(const onKeyCallback& callback)
{
    m_Settings.onKey = callback;
    GLCall(glfwSetKeyCallback(m_Window, [](GLFWwindow* window, const int key, const int scancode, const int action, const int mods)
    {
        auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        win->m_Settings.onKey(win, key, scancode, action, mods);
    }))
}

void Window::onScroll(const scrollCallback& callback)
{
    m_Settings.onScroll = callback;
    GLCall(glfwSetScrollCallback(m_Window, [](GLFWwindow* window, const double xoffset, const double yoffset)
    {
        auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        win->m_Settings.onScroll(win, {xoffset, yoffset});
    }))
}

auto Window::onFocus(const focusCallback& callback)
{
    m_Settings.onFocus = callback;
    GLCall(glfwSetWindowFocusCallback(m_Window, [](GLFWwindow* window, const int focused)
    {
        auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));

        if (focused && win->m_Settings.disableCursor)
        {
            GLCall(glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED))
        }
        else
        {
            GLCall(glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL))
        }

        win->m_Settings.onFocus(win, focused);
    }))
}

void Window::bind() const
{
    GLCall(glfwMakeContextCurrent(m_Window))
}

void Window::initGlfw()
{
    if (!glfwInit())
    {
        LOG_ERROR("Could not init glfw");
    }
    else
    {
        GLCall(glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3))
        GLCall(glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3))
        GLCall(glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE))
    }

    s_glfwInitialized = true;
}