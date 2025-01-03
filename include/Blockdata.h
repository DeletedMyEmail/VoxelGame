#pragma once

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

#define TEST_TEXTURE_OFFSET glm::uvec2{0,0}
#define GRASS_TEXTURE_OFFSET glm::uvec2{1,0}
#define STONE_TEXTURE_OFFSET glm::uvec2{1,1}

enum BlockType {
    SOLID,
    TRANSPARENT,
    INVISIBLE
};

struct Blockdata
{
    BlockType type;
    glm::uvec2 atlasOffset;
};