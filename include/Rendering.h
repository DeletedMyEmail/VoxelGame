#pragma once

#include "VertexArray.h"
#include "Camera.h"
#include "Metrics.h"

void renderConfig(GLFWwindow* window);
void drawAxes(const Camera& cam);
void drawHighlightBlock(const glm::uvec3& positionInChunk, const glm::ivec3& globalOffset, const glm::mat4& viewProjection, float exposure);
void drawChunk(const VertexArray& vao, const glm::ivec3& globalOffset, const glm::mat4& viewProjection, float exposure);
void clearFrame(float skyExposure, bool debugMode);
void drawDebugMenu(const Metrics& metrics, float& exposure, float& camSpeed, const Camera& cam, const std::array<const char*, 5>& comboSelection, int32_t& comboIndex);
void renderCleanup();