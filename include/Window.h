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

class Window
{
public:
  Window();
  Window(int width, int height);
  ~Window();

  void bind() const;
  bool isKeyDown(int pKey) const;
  bool isMouseButtonDown(int pButton) const;
  bool isRunning() const;
  void stop() const;

  void setVSync(bool enabled);
  void setCursorDisabled(bool disabled);
  void onCursorMove(const cursorCallback& callback);
  void onClose(const closeCallback& callback);
  void onMouseButton(const mouseButtonCallback& callback);
  void onKey(const onKeyCallback& callback);
  auto onFocus(const focusCallback& callback);
  void onScroll(const scrollCallback& callback);

  void setTitle(const std::string& title) const;
  glm::dvec2 getMousePosition() const;
  double getMouseX() const;
  double getMouseY() const;
  GLFWwindow* getGLFWWindow() const { return m_Window; }
  int getWidth() const { return m_Width; }
  int getHeight() const { return m_Height; }
private:
  void init();
  void createGLFWWindow();
  static void initGlfw();
private:
  GLFWwindow* m_Window = nullptr;

  const char* m_Title = "";
  int m_Width = 0;
  int m_Height = 0;
  bool m_Fullscreen = false;
  bool m_Vsync = false;
  bool m_DisableCursor = false;

  cursorCallback m_CursorMoveFunc = nullptr;
  closeCallback m_CloseFunc = nullptr;
  mouseButtonCallback m_MouseButtonFunc = nullptr;
  onKeyCallback m_KeyChangeFunc = nullptr;
  focusCallback m_FocusFunc = nullptr;
  scrollCallback m_ScrollFunc = nullptr;

  static bool s_glfwInitialized;
};