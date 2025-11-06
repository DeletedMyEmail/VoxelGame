#include "../include/Rendering.h"
#include <algorithm>
#include <numeric>
#include "Block.h"
#include "imgui.h"
#include "OpenGLHelper.h"
#include "Shader.h"
#include "Texture.h"

VertexArray createHighlightVAO();

Renderer::Renderer()
    :   m_BasicShader("../resources/shaders/BasicVert.glsl", "../resources/shaders/BasicFrag.glsl"),
        m_BlockShader("../resources/shaders/BlockVert.glsl", "../resources/shaders/BlockFrag.glsl"),
        m_HighlightVao(createHighlightVAO()),
        m_TextureAtlas("../resources/textures/TextureAtlas.png")
{
    GLCall(glEnable(GL_CULL_FACE));
    GLCall(glCullFace(GL_FRONT));
    GLCall(glEnable(GL_DEPTH_TEST));
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

Renderer::~Renderer() = default;

void Renderer::drawEntity(const VertexArray& vao, const glm::vec3& pos, const glm::mat4& viewProjection, const float exposure)
{
    vao.bind();
    m_BasicShader.bind();

    m_BasicShader.setUniformMat4("u_VP", viewProjection);
    m_BasicShader.setUniform3f("u_GlobalPosition", pos);
    GLCall(glDrawArrays(GL_LINES, 0, vao.vertexCount));
}

void Renderer::prepareChunkRendering(const glm::mat4& viewProjection, const float exposure)
{
    m_TextureAtlas.bind(0);
    m_BlockShader.bind();
    m_BlockShader.setUniformMat4("u_VP", viewProjection);
    m_BlockShader.setUniform1i("u_textureSlot", 0);
    m_BlockShader.setUniform3f("u_exposure", glm::vec3{exposure});
}

void Renderer::drawChunk(const VertexArray& vao, const glm::ivec3& globalOffset)
{
    if (vao.vertexCount == 0)
        return;

    m_BlockShader.setUniform3f("u_chunkOffset", glm::vec3(globalOffset));
    vao.bind();
    GLCall(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, vao.vertexCount));
}

void Renderer::drawHighlightBlock(const glm::vec3& pos, const glm::mat4& viewProjection, const float exposure)
{
    GLCall(glEnable(GL_DEPTH_TEST));
    GLCall(glDepthFunc(GL_LEQUAL));

    m_HighlightVao.bind();
    m_TextureAtlas.bind(0);
    m_BlockShader.bind();

    m_BlockShader.setUniformMat4("u_VP", viewProjection);
    m_BlockShader.setUniform1i("u_textureSlot", 0);
    m_BlockShader.setUniform3f("u_chunkOffset", pos);
    m_BlockShader.setUniform3f("u_exposure", glm::vec3(exposure));

    GLCall(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, m_HighlightVao.vertexCount));
    GLCall(glDepthFunc(GL_LESS));
}

void Renderer::clearFrame(const float skyExposure) const
{
    glClearColor(0.5f * skyExposure, 0.8f * skyExposure, 0.9f * skyExposure, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

VertexArray createHighlightVAO()
{
    static std::array<blockdata, 6> buffer;
    const glm::uvec2 atlasOffset = getAtlasOffset(BLOCK_TYPE::HIGHLIGHTED, (FACE) 0);

    for (uint32_t i = 0; i < buffer.size(); i++)
        buffer[i] = packBlockData(glm::uvec3(0), atlasOffset, (FACE) i);

    VertexArray highlightVao;
    VertexBufferLayout highlightLayout;
    highlightLayout.pushUInt(1, false, 1);
    highlightVao.addBuffer(createBuffer(buffer.data(), sizeof(blockdata) * buffer.size()), highlightLayout);
    highlightVao.vertexCount = buffer.size();

    return highlightVao;
}