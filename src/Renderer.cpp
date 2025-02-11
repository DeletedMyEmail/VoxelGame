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

Renderer::Renderer()
    :   m_TextureAtlas("../resources/textureAtlas.png"),
        m_ChunkShader("../shader/ChunkVert.glsl", "../shader/ChunkFrag.glsl", nullptr),
        m_BasicShader("../shader/BasicVert.glsl", "../shader/BasicFrag.glsl", nullptr)
{
    GLCall(glEnable(GL_DEPTH_TEST))
    GLCall(glEnable(GL_BLEND))
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA))
    GLCall(glCullFace(GL_FRONT))
    GLCall(glEnable(GL_CULL_FACE))
}

Renderer::~Renderer() = default;

void Renderer::drawChunk(Chunk& chunk, const Window& window, const Camera& cam) const
{
    GLCall(glEnable(GL_CULL_FACE))

    window.bind();
    m_TextureAtlas.bind(0);
    m_ChunkShader.bind();
    const VertexArray& vao = chunk.getMesh();
    vao.bind();

    m_ChunkShader.setUniform1i("u_TextureSlot", 0);
    m_ChunkShader.setUniformMat4("u_MVP", cam.getViewProjection());
    m_ChunkShader.setUniform2u("u_ChunkPos", chunk.getPosition().x, chunk.getPosition().y);

    GLCall(glDrawArrays(GL_TRIANGLES, 0, vao.getVertexCount()))
}

void Renderer::clear(const Window& window, const glm::vec4& color) const
{
    window.bind();

    GLCall(glfwSwapBuffers(window.getGLFWWindow()))
    GLCall(glfwPollEvents())

    GLCall(glClearColor(color.r, color.g, color.b, color.a))
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
}

void Renderer::draw(const VertexArray& vao, const DRAW_GEOMETRY geo, const glm::vec3& position, const Window& window, const Camera& cam) const
{
    GLCall(glDisable(GL_CULL_FACE))
    window.bind();
    vao.bind();

    m_BasicShader.bind();
    m_BasicShader.setUniformMat4("u_MVP", cam.getViewProjection());
    m_BasicShader.setUniform3f("u_GlobalPosition", position);

    GLCall(glDrawArrays(geo, 0, vao.getVertexCount()));
}
