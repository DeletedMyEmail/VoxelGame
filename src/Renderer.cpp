#include "Renderer.h"
#include "Log.h"

void checkOpenGLErrors()
{
    GLenum error = GL_NO_ERROR;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        LOG_ERROR("OpenGL error: " + std::to_string(error));
    }
}

void draw(Chunk& chunk, const Shader& shader, const Texture& texture, const Window& window, const Camera& cam)
{
    window.bind();
    texture.bind(0);
    shader.bind();
    auto[vao, faceCount] = chunk.getMesh();
    vao->bind();

    shader.setUniform1i("u_TextureSlot", 0);
    shader.setUniform4f("u_MVP", cam.getProjection() * cam.getView());
    shader.setUniform2u("u_ChunkPos", chunk.getPosition().x, chunk.getPosition().y);

    GLCall(glDrawArrays(GL_TRIANGLES, 0, faceCount * 6))
}

void clear(const Window& window, const glm::vec4 color)
{
    window.bind();

    GLCall(glfwSwapBuffers(window.getGLFWWindow()))
    GLCall(glfwPollEvents())

    GLCall(glClearColor(color.r, color.g, color.b, color.a))
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
}