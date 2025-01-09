#include "Window.h"
#include "Log.h"
#include "Renderer.h"

bool Window::s_glfwInitialized = false;

Window::Window(const WindowSettings&settings)
    : m_Window(nullptr), m_Settings(settings)
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

    GLCall(glEnable(GL_DEPTH_TEST))
    GLCall(glEnable(GL_BLEND))
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA))

    //GLCall(glfwSetCursorPos(m_Window, settings.height / 2, settings.width / 2))
    GLCall(glfwSetInputMode(m_Window, GLFW_CURSOR, settings.disableCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL))

    if (settings.culling)
    {
        GLCall(glEnable(GL_CULL_FACE))
    }
    if (!settings.vysnc)
    {
        GLCall(glfwSwapInterval(0))
    }

    setCallbacks();
}

Window::Window(Window&& other) noexcept
{
    m_Window = other.m_Window;
    m_Settings = other.m_Settings;
    other.m_Window = nullptr;
}

Window::~Window() {
    GLCall(glfwDestroyWindow(m_Window))
}

void Window::createGLFWWindow()
{
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    if (m_Settings.fullscreen) {
        m_Window = glfwCreateWindow(mode->width, mode->height, m_Settings.title, glfwGetPrimaryMonitor(), nullptr);
        m_Settings.width = mode->width;
        m_Settings.height = mode->height;
    }
    else {
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

void Window::setCallbacks() const
{
    GLCall(glfwSetWindowFocusCallback(m_Window, windowFocusCallback))
    GLCall(glfwSetWindowCloseCallback(m_Window, closeCallback))
    GLCall(glfwSetMouseButtonCallback(m_Window, mouseButtonCallback))
    GLCall(glfwSetKeyCallback(m_Window, keyCallback))
    GLCall(glfwSetCursorPosCallback(m_Window, mouseMoveCallback))
    GLCall(glfwSetScrollCallback(m_Window, scrollCallback))
}

void Window::windowFocusCallback(GLFWwindow* window, const int focused)
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

    if (win->m_Settings.onFocusCallback)
        win->m_Settings.onFocusCallback(win, focused);
}

void Window::mouseMoveCallback(GLFWwindow* window, const double x, const double y)
{
    auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (win->m_Settings.onCursorMoveCallback)
        win->m_Settings.onCursorMoveCallback(win, glm::dvec2{x,y});
}

void Window::closeCallback(GLFWwindow* window)
{
    auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (win->m_Settings.onCloseCallback)
        win->m_Settings.onCloseCallback(win);
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (win->m_Settings.onFocusCallback)
        win->m_Settings.onMouseButtonCallback(win, button, action, mods);
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (win->m_Settings.onKeyCallback)
        win->m_Settings.onKeyCallback(win, key, scancode, action, mods);
}

void Window::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (win->m_Settings.onScrollCallback)
        win->m_Settings.onScrollCallback(win, xoffset, yoffset);
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

// Builder -----------------------------------

WindowBuilder& WindowBuilder::vsync(const bool enable)
{
    m_settings.vysnc = enable;
    return *this;
}

WindowBuilder& WindowBuilder::fullscreen(const bool enable)
{
    m_settings.fullscreen = enable;
    return *this;
}

WindowBuilder& WindowBuilder::culling(const bool enable)
{
    m_settings.culling = enable;
    return *this;
}

WindowBuilder& WindowBuilder::disableCursor(const bool enable)
{
    m_settings.disableCursor = enable;
    return *this;
}

WindowBuilder& WindowBuilder::title(const char* title)
{
    m_settings.title = title;
    return *this;
}

WindowBuilder& WindowBuilder::size(int height, int width)
{
    m_settings.height = height;
    m_settings.width = width;
    return *this;
}

WindowBuilder& WindowBuilder::onCursorMove(const std::function<void(Window* window, glm::dvec2 pos)>& callback)
{
    m_settings.onCursorMoveCallback = callback;
    return *this;
}

WindowBuilder& WindowBuilder::onKey(
    const std::function<void(Window* window, int key, int scancode, int action, int mods)>& callback)
{
    m_settings.onKeyCallback = callback;
    return *this;
}

WindowBuilder& WindowBuilder::onFocus(const std::function<void(Window* window, bool focused)>& callback)
{
    m_settings.onFocusCallback = callback;
    return *this;
}

WindowBuilder& WindowBuilder::onMouseButton(
    const std::function<void(Window* window, int button, int action, int mods)>& callback)
{
    m_settings.onMouseButtonCallback = callback;
    return *this;
}

WindowBuilder& WindowBuilder::onClose(const std::function<void(Window* window)>& callback)
{
    m_settings.onCloseCallback = callback;
    return *this;
}

WindowBuilder& WindowBuilder::onScroll(
    const std::function<void(Window* window, double xoffset, double yoffset)>& callback)
{
    m_settings.onScrollCallback = callback;
    return *this;
}

Window WindowBuilder::build() const
{
    return {m_settings};
}
