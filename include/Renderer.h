#pragma once

#include "Camera.h"
#include "glad/glad.h"
#include "Log.h"
#include "Mesh.h"
#include "Shader.h"
#include "Window.h"

#ifndef RELEASE
    #define GLCall(x) x; checkOpenGLErrors();
#else
    #define GLCall(x) x;
#endif


inline void checkOpenGLErrors() {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        LOG_ERROR(error);
    }
}

class Renderer
{
public:
    Renderer(const std::shared_ptr<Window>& window, std::shared_ptr<Camera> cam);

    void clear(glm::vec4 color = {0.07f, 0.14f, 0.17f, 1.0f}) const;
    void draw(const Mesh& mesh, const Texture& texture, const Shader& shader) const;
    void update() const;

    std::shared_ptr<Window> getWindow() { return m_Window; }
    std::shared_ptr<Camera> getCamera() { return m_Cam; }
private:
    std::shared_ptr<Window> m_Window;
    std::shared_ptr<Camera> m_Cam;
};