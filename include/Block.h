#pragma once

#include "glad/glad.h"
#include "glm/vec2.hpp"

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
    TEST,
    INVALID,
    AIR,
    GRASS,
    GRASS_FULL,
    STONE,
    SAND
};

typedef GLuint blockdata;

inline glm::uvec2 getAtlasOffset(const BLOCK_TYPE block, const uint32_t face)
{
    switch (block)
    {
        case BLOCK_TYPE::TEST:
                return {0+face,0};
        case BLOCK_TYPE::GRASS:
            return {0+face,1};
        case BLOCK_TYPE::GRASS_FULL:
            return {6+face,1};
        case BLOCK_TYPE::STONE:
            return {0+face,2};
        case BLOCK_TYPE::SAND:
            return {6+face,2};
        default:
            return {0,0};
    }
}