#include "Renderer.h"
#include "Log.h"
#define GLT_IMPLEMENTATION
#define GLT_MANUAL_VIEWPORT
#include <glText/gltext.h>

void checkOpenGLErrors()
{
    GLenum error = GL_NO_ERROR;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        LOG_ERROR("OpenGL error: " + std::to_string(error));
    }
}

Renderer::Renderer(const int width, const int height)
    :   m_TextureAtlas("../resources/textureAtlas.png"),
        m_ChunkShader("../shader/ChunkVert.glsl", "../shader/ChunkFrag.glsl", nullptr),
        m_BasicShader("../shader/BasicVert.glsl", "../shader/BasicFrag.glsl", nullptr)
{
    GLCall(glEnable(GL_DEPTH_TEST))
    GLCall(glEnable(GL_BLEND))
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA))
    GLCall(glCullFace(GL_FRONT))
    GLCall(glEnable(GL_CULL_FACE))

    if (!gltInit())
    {
        LOG_ERROR("Failed to initialize glText");
    }
    gltViewport(width, height);
}

Renderer::~Renderer()
{
    gltTerminate();
};

void Renderer::drawChunk(Chunk& chunk, const Camera& cam) const
{
    GLCall(glEnable(GL_CULL_FACE))

    m_TextureAtlas.bind(0);
    m_ChunkShader.bind();
    const VertexArray& vao = chunk.getMesh();
    vao.bind();

    m_ChunkShader.setUniform1i("u_TextureSlot", 0);
    m_ChunkShader.setUniformMat4("u_MVP", cam.getViewProjection());
    m_ChunkShader.setUniform2u("u_ChunkPos", chunk.getPosition().x, chunk.getPosition().y);

    GLCall(glDrawArrays(GL_TRIANGLES, 0, vao.getVertexCount()))
}

void Renderer::update(const Window& window, const glm::vec4& color) const
{
    GLCall(glfwSwapBuffers(window.getGLFWWindow()))
    GLCall(glfwPollEvents())

    GLCall(glClearColor(color.r, color.g, color.b, color.a))
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
}

void Renderer::draw(const VertexArray& vao, const DRAW_GEOMETRY geo, const glm::vec3& position, const Camera& cam) const
{
    GLCall(glDisable(GL_CULL_FACE))
    vao.bind();

    m_BasicShader.bind();
    m_BasicShader.setUniformMat4("u_MVP", cam.getViewProjection());
    m_BasicShader.setUniform3f("u_GlobalPosition", position);

    GLCall(glDrawArrays(geo, 0, vao.getVertexCount()));
}

void Renderer::draw(const char* text, const glm::ivec2& position, const float scale) const
{
    GLCall(glFrontFace(GL_CW))

    GLTtext* glText = gltCreateText();
    gltSetText(glText, text);
    gltBeginDraw();
    gltColor(1.0f, 1.0f, 1.0f, 1.0f);
    gltDrawText2D(glText, position.x, position.y, scale);
    gltEndDraw();
    gltDeleteText(glText);

    GLCall(glFrontFace(GL_CCW))
}
