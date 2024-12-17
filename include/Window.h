#pragma once

#include <functional>
#include "../libs/glad/glad.h"
#include <GLFW/glfw3.h>
#include "../libs/glm/vec4.hpp"
#include "../libs/glm/vec2.hpp"
#include "Camera.h"

class Window;

typedef struct WindowSetting
{
  int width = 0;
  int height = 0;
  const char* title = nullptr;
  bool fullscreen = false;
  bool culling = false;
  bool vysnc = false;
  bool disableCursor = false;

  std::function<void(Window* window, glm::dvec2 pos)> onCursorMoveCallback = nullptr;
  std::function<void(Window* window)> onCloseCallback = nullptr;
  std::function<void(Window* window, int button, int action, int mods)> onMouseButtonCallback = nullptr;
  std::function<void(Window* window, int key, int scancode, int action, int mods)> onKeyCallback = nullptr;
  std::function<void(Window* window, bool focused)> onFocusCallback = nullptr;
} WindowSettings;

class Window
{
public:
  explicit Window(const WindowSettings& settings);
  ~Window();
  void clear(glm::vec4 color = {0.07f, 0.14f, 0.17f, 1.0f}) const;

  bool isRunning() const;
  void stop();

  glm::dvec2 getMousePosition() const;
  double getMouseX() const;
  double getMouseY() const;
  bool isKeyPressed(int pKey) const;
  bool isMouseButtonPressed(int pButton) const;

  const WindowSetting& getSettings() const { return m_Settings; }
  GLFWwindow* getGLFWWindow() const { return m_Window; }
  Camera& getCamera() { return m_Camera; }

private:
  void setCallbacks() const;
  void createGLFWWindow();

  static void windowFocusCallback(GLFWwindow* window, int focused);
  static void mouseMoveCallback(GLFWwindow* window, double x, double y);
  static void closeCallback(GLFWwindow* window);
  static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void initGlfw();
private:
  GLFWwindow* m_Window;
  WindowSetting m_Settings;
  Camera m_Camera;

  static bool s_glfwInitialized;
};

class WindowBuilder
{
public:
  WindowBuilder() = default;

  WindowBuilder& vsync(bool enable = true);
  WindowBuilder& fullscreen(bool enable = true);
  WindowBuilder& culling(bool enable = true);
  WindowBuilder& disableCursor(bool enable = true);
  WindowBuilder& title(const char* title);
  WindowBuilder& size(int height, int width);

  WindowBuilder& onCursorMove(const std::function<void(Window* window, glm::dvec2 pos)>& callback);
  WindowBuilder& onKey(const std::function<void(Window* window, int key, int scancode, int action, int mods)>& callback);
  WindowBuilder& onFocus(const std::function<void(Window* window, bool focused)>& callback);
  WindowBuilder& onMouseButton(const std::function<void(Window* window, int button, int action, int mods)>& callback);
  WindowBuilder& onClose(const std::function<void(Window* window)>& callback);

  Window build() const;

  const WindowSetting& getSettings() const { return m_settings; }
private:
  WindowSetting m_settings;
};