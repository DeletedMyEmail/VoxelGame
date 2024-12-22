#include "Renderer.h"

Renderer::Renderer(const std::shared_ptr<Window>& window, std::shared_ptr<Camera> cam)
    : m_Window(window), m_Cam(cam)
{
}

void Renderer::draw(const Mesh& mesh, const Texture& texture, const Shader& shader) const
{
    shader.bind();
    texture.bind(0);
    shader.setUniform1i("u_Texture", 0);
    shader.setUniform4f("u_View", m_Cam->getView());
    shader.setUniform4f("u_Projection", m_Cam->getProjection());
    mesh.bind();

    GLCall(glDrawElements(GL_TRIANGLES, mesh.getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr))
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


/*GLuint instanceBuffer = 0;
// instance rendering
glm::vec2 translations[BLOCK_COUNT];
for(int i = 0; i < BLOCK_COUNT; i++)
{
    translations[i] = {10.0f * i, 10.0f * i};
}
glGenBuffers(1, &instanceBuffer);
glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * BLOCK_COUNT, translations, GL_STATIC_DRAW);

glEnableVertexAttribArray(2);
glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
glVertexAttribDivisor(2, 1);*/
