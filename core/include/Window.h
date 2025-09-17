#pragma once

#include <functional>
#include <memory>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "glm/vec4.hpp"
#include "glm/vec2.hpp"

struct WindowSettings
{
    int width = 1280;
    int height = 720;
    const char* title = "OpenGL Window";
    bool fullscreen = false;
    bool vSync = false;
    bool disableCursor = false;
};

class Window
{
public:
    Window() = default;
    Window(WindowSettings& settings);
    ~Window();

    Window& operator=(Window&& other);
    Window& operator=(const Window& other) = delete;

    void destroy();
    void bind() const;
    void disableCursor(bool disable = true) const;
    bool shouldClose() const;
    glm::dvec2 getMousePosition() const;
    bool isKeyDown(int key) const;
    bool isMouseButtonDown(int button) const;

    const WindowSettings& getSettings() { return m_Settings; }
    GLFWwindow* getHandle() const { return m_Handle; }
private:
    GLFWwindow* m_Handle = nullptr;
    WindowSettings m_Settings;
};