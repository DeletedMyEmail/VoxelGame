#pragma once

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

#define GRASS_TEXTURE_OFFSET glm::uvec2{1,0}
#define STONE_TEXTURE_OFFSET glm::uvec2{1,1}

enum BlockType {
    SOLID,
    TRANSPARENT,
    LIQUID,
    AIR
};

struct Blockdata
{
    BlockType type;
    glm::uvec2 atlasOffset;
    glm::uvec3 pos;
};