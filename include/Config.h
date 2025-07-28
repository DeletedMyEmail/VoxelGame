#pragma once
#include <thread>

#include "WorldGeneration.h"

namespace config
{
    const int32_t RENDER_DISTANCE = 8;
    const int32_t LOAD_DISTANCE = RENDER_DISTANCE * 2;

    const int32_t MAX_LOADS_PER_FRAME = 32;
    const int32_t MAX_UNLOADS_PER_FRAME = 32;
    const int32_t LOADING_THREADS = 16;
    const int32_t MAX_BAKES_PER_FRAME = 1;

    const BIOME WORLD_BIOME = HILLS;
    const int32_t WORLD_SEED = 1337;

    const float REACH_DISTANCE = 16.0f;
}
