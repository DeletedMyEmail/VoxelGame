#pragma once

#include "VertexArray.h"
#include "Camera.h"
#include "Config.h"
#include "Metrics.h"

void renderConfig(GLFWwindow* window);
void drawAxes(const Camera& cam);
void drawHighlightBlock(const glm::uvec3& positionInChunk, const glm::ivec3& globalOffset, const glm::mat4& viewProjection, float exposure);
void drawChunk(const VertexArray& vao, const glm::ivec3& globalOffset, const glm::mat4& viewProjection, float exposure);
void clearFrame(float skyExposure, bool debugMode);
void drawDebugMenu(const Metrics& metrics, MenuSettings& settings, const glm::vec3& pos, const ProgramConfig& config);
void renderCleanup();