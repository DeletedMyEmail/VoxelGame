#pragma once

#include "VertexArray.h"
#include "Camera.h"
#include "Config.h"
#include "Metrics.h"
#include "Shader.h"
#include "Texture.h"
#include "GLFW/glfw3.h"

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void prepareChunkRendering(const glm::mat4& viewProjection, float exposure);
    void drawHighlightBlock(const glm::vec3& pos, const glm::mat4& viewProjection, float exposure);
    void drawChunk(const VertexArray& vao, const glm::ivec3& globalOffset);
    void clearFrame(float skyExposure) const;
    void drawEntity(const VertexArray& vao, const glm::vec3& pos, const glm::mat4& viewProjection, float exposure);
private:
    Shader m_BasicShader, m_BlockShader;
    VertexArray m_HighlightVao;
    Texture m_TextureAtlas;
};
