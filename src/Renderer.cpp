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
        m_DefaultShader("../shader/DefaultVert.glsl", "../shader/DefaultFrag.glsl", nullptr),
        m_DebugShader("../shader/DebugVert.glsl", "../shader/DebugFrag.glsl", nullptr)
{
    GLCall(glEnable(GL_DEPTH_TEST))
    GLCall(glEnable(GL_BLEND))
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA))
    GLCall(glEnable(GL_CULL_FACE))
}

Renderer::~Renderer() = default;

void Renderer::drawChunk(Chunk& chunk, const Window& window, const Camera& cam)
{
    window.bind();
    m_TextureAtlas.bind(0);
    m_DefaultShader.bind();
    auto[vao, faceCount] = chunk.getMesh();
    vao->bind();

    m_DefaultShader.setUniform1i("u_TextureSlot", 0);
    m_DefaultShader.setUniformMat4("u_MVP", cam.getViewProjection());
    m_DefaultShader.setUniform2u("u_ChunkPos", chunk.getPosition().x, chunk.getPosition().y);

    GLCall(glDrawArrays(GL_TRIANGLES, 0, faceCount * 6))
}

void Renderer::clear(const Window& window, const glm::vec4 color)
{
    window.bind();

    GLCall(glfwSwapBuffers(window.getGLFWWindow()))
    GLCall(glfwPollEvents())

    GLCall(glClearColor(color.r, color.g, color.b, color.a))
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
}

void Renderer::drawAxes(const Window& window, const Camera& cam)
{
    window.bind();

    constexpr float axisVertices[] =
    {
        // X axis
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

        // Y axis
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f,

        // Z axis
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f
    };

    const auto vBuffer = std::make_shared<VertexBuffer>(6 * 6 * sizeof(float), axisVertices);
    VertexBufferLayout layout;
    layout.push<float>(3);
    layout.push<float>(3);

    VertexArray VAO;
    VAO.addBuffer(vBuffer, layout);
    VAO.bind();

    m_DebugShader.bind();
    m_DebugShader.setUniformMat4("u_MVP", cam.getViewProjection());
    m_DebugShader.setUniform3f("u_GlobalPosition", cam.getPosition() + cam.getLookDir());

    GLCall(glDrawArrays(GL_LINES, 0, 6));
}

void Renderer::drawPlayer(const glm::vec3 position, const Window& window, const Camera& cam)
{
    window.bind();

    constexpr float playerVertices[] =
    {
        // bottom
         0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 1.0f, // br
        -0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 1.0f, // bl
         0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f, // tr

        -0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f, // tl
         0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f, // tr
        -0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 1.0f, // bl

        // right side
        -0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 1.0f, // bl
        -0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f, // tl
        -0.5f,  1.5f, -0.5f,   1.0f, 1.0f, 1.0f, // br

        -0.5f,  1.5f,  0.5f,   1.0f, 1.0f, 1.0f, // tr
        -0.5f,  1.5f, -0.5f,   1.0f, 1.0f, 1.0f, // br
        -0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f, // tl

        // left side
         0.5f,  1.5f, -0.5f,   1.0f, 1.0f, 1.0f, // br
         0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f, // tl
         0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 1.0f, // bl

         0.5f,  1.5f, -0.5f,   1.0f, 1.0f, 1.0f, // br
         0.5f,  1.5f,  0.5f,   1.0f, 1.0f, 1.0f, // tr
         0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f, // tl

        // top
         0.5f,  1.5f, -0.5f,   1.0f, 1.0f, 1.0f, // br
        -0.5f,  1.5f, -0.5f,   1.0f, 1.0f, 1.0f, // bl
         0.5f,  1.5f,  0.5f,   1.0f, 1.0f, 1.0f, // tr

        -0.5f,  1.5f,  0.5f,   1.0f, 1.0f, 1.0f, // tl
         0.5f,  1.5f,  0.5f,   1.0f, 1.0f, 1.0f, // tr
        -0.5f,  1.5f, -0.5f,   1.0f, 1.0f, 1.0f, // bl

        // front
         0.5f, -0.5f,  -0.5f,   1.0f, 1.0f, 1.0f, // br
        -0.5f, -0.5f,  -0.5f,   1.0f, 1.0f, 1.0f, // bl
         0.5f,  1.5f,  -0.5f,   1.0f, 1.0f, 1.0f, // tr

        -0.5f,  1.5f,  -0.5f,   1.0f, 1.0f, 1.0f, // tl
         0.5f,  1.5f,  -0.5f,   1.0f, 1.0f, 1.0f, // tr
        -0.5f, -0.5f,  -0.5f,   1.0f, 1.0f, 1.0f, // bl
    };

    const auto vBuffer = std::make_shared<VertexBuffer>(30 * 6 * sizeof(float), playerVertices);
    VertexBufferLayout layout;
    layout.push<float>(3);
    layout.push<float>(3);

    VertexArray VAO;
    VAO.addBuffer(vBuffer, layout);
    VAO.bind();

    m_DebugShader.bind();
    m_DebugShader.setUniformMat4("u_MVP", cam.getViewProjection());
    m_DebugShader.setUniform3f("u_GlobalPosition", position);

    GLCall(glDrawArrays(GL_TRIANGLES, 0, 30));
}
