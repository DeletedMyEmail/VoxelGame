#pragma once
#include "Chunk.h"
#include "glm/vec3.hpp"

struct RaycastResult
{
    glm::uvec3 pos;
    Chunk* chunk;
    BLOCK_TYPE blockType;
    FACE face;
    bool hit;
};

RaycastResult raycast(const glm::vec3& origin, const glm::vec3& dir, float radius, std::unordered_map<uint64_t, Chunk>& chunks);