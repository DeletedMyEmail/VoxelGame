#pragma once

#include "glad/glad.h"
#include "glm/vec2.hpp"

enum FACES : uint8_t
{
    BACK = 0,
    FRONT = 1,
    LEFT = 2,
    RIGHT = 3,
    TOP = 4,
    BOTTOM = 5
};

enum class BLOCK_TYPE : uint8_t
{
    TEST,
    INVALID,
    AIR,
    GRASS,
    STONE,
};

typedef GLuint blockdata;

inline glm::uvec2 getAtlasOffset(const BLOCK_TYPE block, uint32_t face)
{
    switch (block)
    {
    case BLOCK_TYPE::GRASS:
        return {6+face,0};
    case BLOCK_TYPE::STONE:
        return {6+face,1};
    default:
        return {face,0};
    }
}