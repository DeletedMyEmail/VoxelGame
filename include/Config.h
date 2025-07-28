#pragma once

#include "WorldGeneration.h"

namespace config
{
    const int32_t RENDER_DISTANCE = 8;
    const int32_t LOAD_DISTANCE = RENDER_DISTANCE * 2;

    const int32_t MAX_LOADS_PER_FRAME = 32;
    const int32_t MAX_UNLOADS_PER_FRAME = MAX_LOADS_PER_FRAME;
    const int32_t THREAD_COUNT = std::thread::hardware_concurrency();
    const int32_t MAX_BAKES_PER_FRAME = 8;

    const BIOME WORLD_BIOME = HILLS;
    const int32_t WORLD_SEED = 42;

    const float REACH_DISTANCE = 16.0f;
}
