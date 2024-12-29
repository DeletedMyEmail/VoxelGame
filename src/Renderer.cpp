#include "Renderer.h"

void checkOpenGLErrors()
{
    GLenum error = GL_NO_ERROR;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        LOG_ERROR(error);
    }
}

static Mesh createBlockMesh();

BlockRenderer::BlockRenderer(const char* textureAtlasPath)
    : m_BlockMesh(createBlockMesh()), m_TextureAtlas(textureAtlasPath)
{
}

void BlockRenderer::draw(const Chunk& chunk, const Shader& shader, const Window& window, const Camera& cam) const
{
    window.bind();
    m_TextureAtlas.bind(0);
    shader.bind();

    shader.setUniform1i("u_TextureSlot", 0);
    shader.setUniform2u("u_AtlasCoords", 1, 11);
    shader.setUniform4f("u_View", cam.getView());
    shader.setUniform4f("u_Projection", cam.getProjection());

    m_BlockMesh.bind();

    GLCall(glDrawElements(GL_TRIANGLES, m_BlockMesh.getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr))
}

void BlockRenderer::clear(const Window& window, const glm::vec4 color) const
{
    window.bind();

    GLCall(glfwSwapBuffers(window.getGLFWWindow()))
    GLCall(glfwPollEvents())

    GLCall(glClearColor(color.r, color.g, color.b, color.a))
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
}

static Mesh createBlockMesh()
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

    const std::vector<GLsizei> indices = {
        0, 1, 2, 2, 3, 0,       // Front face
        4, 5, 6, 6, 7, 4,       // Back face
        8, 9, 10, 10, 11, 8,    // Left face
        12, 13, 14, 14, 15, 12, // Right face
        16, 17, 18, 18, 19, 16, // Top face
        20, 21, 22, 22, 23, 20  // Bottom face
    };

    const std::shared_ptr<VertexBuffer> vBuffer = std::make_unique<VertexBuffer>(sizeof(GLuint) * vertices.size(), vertices.data());
    const std::shared_ptr<IndexBuffer> iBuffer = std::make_unique<IndexBuffer>(indices.data(), indices.size());

    VertexBufferLayout vertLayout;
    vertLayout.push<GLuint>(1);

    const std::shared_ptr<VertexArray> vArray = std::make_unique<VertexArray>();
    vArray->addBuffer(vBuffer, vertLayout);

    return {vArray, iBuffer};
}
