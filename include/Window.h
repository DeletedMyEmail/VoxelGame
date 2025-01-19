#pragma once

#include <functional>
#include <memory>

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "glm/vec4.hpp"
#include "glm/vec2.hpp"

class Window;

typedef std::function<void(Window* window, glm::dvec2 offset)> scrollCallback;
typedef std::function<void(Window* window, bool focused)> focusCallback;
typedef std::function<void(Window* window, int key, int scancode, int action, int mods)> onKeyCallback;
typedef std::function<void(Window* window, int button, int action, int mods)> mouseButtonCallback;
typedef std::function<void(Window* window)> closeCallback;
typedef std::function<void(Window* window, glm::dvec2 pos)> cursorCallback;

struct WindowSettings
{
  int width = 0;
  int height = 0;
  const char* title = "";
  bool fullscreen = false;
  bool vysnc = false;
  bool disableCursor = false;

  cursorCallback onCursorMove = nullptr;
  closeCallback onClose = nullptr;
  mouseButtonCallback onMouseButton = nullptr;
  onKeyCallback onKey = nullptr;
  focusCallback onFocus = nullptr;
  scrollCallback onScroll = nullptr;
};

class Window
{
public:
  Window();
  Window(int width, int height);
  ~Window();

  void bind() const;
  bool isKeyDown(int pKey) const;
  bool isMouseButtonPressed(int pButton) const;

  void setVSync(bool enabled);
  void setCursorDisabled(bool disabled);
  void onCursorMove(const cursorCallback& callback);
  void onClose(const closeCallback& callback);
  void onMouseButton(const mouseButtonCallback& callback);
  void onKey(const onKeyCallback& callback);
  auto onFocus(const focusCallback& callback);
  void onScroll(const scrollCallback& callback);

  glm::dvec2 getMousePosition() const;
  double getMouseX() const;
  double getMouseY() const;
  bool isRunning() const;
  void stop() const;
  void setTitle(const std::string& title) const;

  const WindowSettings& getSettings() const { return m_Settings; }
  GLFWwindow* getGLFWWindow() const { return m_Window; }
private:
  void init();
  void createGLFWWindow();
  static void initGlfw();
private:
  GLFWwindow* m_Window = nullptr;
  WindowSettings m_Settings;

  static bool s_glfwInitialized;
};