#include "Renderer.h"

void checkOpenGLErrors()
{
    GLenum error = GL_NO_ERROR;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        LOG_ERROR(error);
    }
}

BlockRenderer::BlockRenderer(const char* textureAtlasPath)
    : m_TextureAtlas(textureAtlasPath)
{
        const std::vector<GLuint> vertices = {
        // 32 bits - 0: pos-x, 1: pos-y, 2: pos-z, 3-6: texture-x, 6: texture-y

        // Front face
        0b1001111u, // Bottom-left
        0b1000011u, // Bottom-right
        0b0000001u, // Top-right
        0b0001101u, // Top-left
        // Back face
        0b1001110u, // Bottom-left
        0b1010010u, // Bottom-right
        0b0010000u, // Top-right
        0b0001100u, // Top-left
        // Left face
        0b1010111u, // Bottom-left
        0b1011110u, // Bottom-right
        0b0011100u, // Top-right
        0b0010101u, // Top-left
        // Right face
        0b1100011u, // Bottom-left
        0b1011010u, // Bottom-right
        0b0011000u, // Top-right
        0b0100001u, // Top-left
        // Top face
        0b1101101u, // Bottom-left
        0b1100001u, // Bottom-right
        0b0100000u, // Top-right
        0b0101100u, // Top-left
        // bottom face
        0b1101111u, // Bottom-left
        0b1110011u, // Bottom-right
        0b0110010u, // Top-right
        0b0101110u, // Top-left
    };

    const std::shared_ptr<VertexBuffer> vBuffer = std::make_unique<VertexBuffer>(sizeof(GLuint) * vertices.size(), vertices.data());

    VertexBufferLayout vertLayout;
    vertLayout.push<GLuint>(1);

    m_BlockVertArray.addBuffer(vBuffer, vertLayout);

}

void BlockRenderer::setInstanceData(Chunk& chunk)
{
    GLuint translations[chunk.getBlocks().size() * 5];
    for (size_t i = 0; i < chunk.getBlocks().size(); i ++)
    {
        translations[i * 5] = chunk.getBlocks()[i].pos.x;
        translations[i * 5 + 1] = chunk.getBlocks()[i].pos.y;
        translations[i * 5 + 2] = chunk.getBlocks()[i].pos.z;
        translations[i * 5 + 3] = chunk.getBlocks()[i].atlasOffset.x;
        translations[i * 5 + 4] = chunk.getBlocks()[i].atlasOffset.y;
    }

    const std::shared_ptr<VertexBuffer> instanceBuffer = std::make_unique<VertexBuffer>(sizeof(GLuint) * 5 * chunk.getBlocks().size(), (void*) translations);

    VertexBufferLayout instanceLayout;
    instanceLayout.push<GLuint>(3, false, 1);
    instanceLayout.push<GLuint>(2, false, 1);

    if (m_BlockVertArray.getBufferCount() < 2)
        m_BlockVertArray.addBuffer(instanceBuffer, instanceLayout);
    else
        m_BlockVertArray.updateBuffer(1, instanceBuffer, instanceLayout);
}

void BlockRenderer::draw(Chunk& chunk, const Shader& shader, const Window& window, const Camera& cam)
{
    window.bind();
    m_BlockIndexBuffer.bind();
    m_TextureAtlas.bind(0);
    shader.bind();

    setInstanceData(chunk);

    shader.setUniform1i("u_TextureSlot", 0);
    shader.setUniform4f("u_View", cam.getView());
    shader.setUniform4f("u_Projection", cam.getProjection());
    shader.setUniform2u("u_ChunkPos", chunk.getPosition().x, chunk.getPosition().y);

    GLCall(glDrawElementsInstanced(GL_TRIANGLES, m_BlockIndexBuffer.getCount(), GL_UNSIGNED_INT, nullptr, chunk.getBlocks().size()))
}

void BlockRenderer::clear(const Window& window, const glm::vec4 color) const
{
    window.bind();

    GLCall(glfwSwapBuffers(window.getGLFWWindow()))
    GLCall(glfwPollEvents())

    GLCall(glClearColor(color.r, color.g, color.b, color.a))
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
}