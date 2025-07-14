#pragma once
#include "Block.h"
#include "FastNoiseLite.h"

enum BIOME
{
    PLAINS = 0,
    DESERT,
    FOREST,
    MOUNTAIN,
    HILLS
};

FastNoiseLite createBiomeNoise(BIOME b, int32_t seed);
BLOCK_TYPE defaultBiomeBlock(BIOME b);
