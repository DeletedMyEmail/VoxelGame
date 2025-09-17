#pragma once

#include "../../core/include/VertexArray.h"
#include "../../core/include/Camera.h"
#include "Config.h"
#include "Metrics.h"
#include "../../core/include/Texture.h"
#include "GLFW/glfw3.h"

struct Renderer
{
    Renderer(GLFWwindow* win);
    ~Renderer();

    void prepareChunkRendering(const glm::mat4& viewProjection, const float exposure) const;
    void drawAxes(const Camera& cam) const;
    void drawHighlightBlock(const glm::vec3& pos, const glm::mat4& viewProjection, float exposure) const;
    void drawChunk(const VertexArray& vao, const glm::ivec3& globalOffset) const;
    void clearFrame(float skyExposure, bool debugMode) const;
    void drawDebugMenu(const Metrics& metrics, MenuSettings& settings, const glm::vec3& pos, const GameConfig& config) const;
    void drawEntity(const VertexArray& vao, const glm::vec3& pos, const glm::mat4& viewProjection, float exposure) const;

    VertexArray axisVao, highlightVao;
    Texture textureAtlas;
    uint32_t basicShader, blockShader;
};
