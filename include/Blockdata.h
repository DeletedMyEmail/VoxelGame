#pragma once
#include "glm/vec3.hpp"

enum BlockType {
    SOLID,
    TRANSPARENT,
    LIQUID,
    AIR
};

constexpr unsigned char ALL_FACES_FREE = 0b111111;

struct Blockdata
{
    BlockType type;
    glm::uvec3 pos; // relative to chunk
};