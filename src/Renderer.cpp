#include "Renderer.h"

#include <utility>

Renderer::Renderer(std::shared_ptr<Window> window, std::shared_ptr<Camera> cam)
    : m_Window(std::move(window)), m_Cam(std::move(cam))
{
}

void Renderer::draw(const Mesh& mesh, const Texture& texture, const Shader& shader, unsigned instanceCount) const
{
    shader.bind();
    texture.bind(0);
    shader.setUniform1i("u_Texture", 0);
    shader.setUniform4f("u_View", m_Cam->getView());
    shader.setUniform4f("u_Projection", m_Cam->getProjection());
    mesh.bind();

    GLCall(glDrawElementsInstanced(GL_TRIANGLES, mesh.getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr, instanceCount))
}

void Renderer::clear(const glm::vec4 color) const
{
    GLCall(glClearColor(color.r, color.g, color.b, color.a))
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
}

void Renderer::update() const
{
    m_Window->bind();
    GLCall(glfwSwapBuffers(m_Window->getGLFWWindow()))
    GLCall(glfwPollEvents())
}