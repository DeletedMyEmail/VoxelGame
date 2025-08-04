#pragma once

#include "cstmlib/Log.h"
#include "glad/glad.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

enum FACE
{
    BACK = 0,
    FRONT = 1,
    LEFT = 2,
    RIGHT = 3,
    BOTTOM = 4,
    TOP = 5,
    INVALID = 6
};

enum class BLOCK_TYPE
{
    INVALID = 0,
    TEST,
    HIGHLIGHTED,
    AIR,
    GRASS,
    GRASS_FULL,
    STONE,
    SAND,
    WOOD,
    LEAVES,
    PUMPKIN,
    MELON,
    WATER
};

typedef GLuint blockdata;
blockdata packBlockData(const glm::uvec3& positionInChunk, const glm::uvec2& atlasOffset, FACE face);
glm::uvec2 getAtlasOffset(BLOCK_TYPE block, FACE face);
