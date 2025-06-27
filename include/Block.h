#pragma once

#include "glad/glad.h"

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