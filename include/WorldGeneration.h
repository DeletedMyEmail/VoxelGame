#pragma once

#include "glm/vec2.hpp"

uint32_t getHeightAt(const glm::ivec2& pos);
bool hasTree(const glm::ivec2& pos);
bool isTreeChunk(const glm::ivec2& pos);

inline const uint32_t WORLD_HEIGHT = 6;
inline const uint32_t SEA_LEVEL = 64.0f;
inline const float MAX_HEIGHT = 300.0f;
inline const float MIN_HEIGHT = 10.0f;