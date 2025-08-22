#pragma once

#include "Block.h"

struct MenuSettings
{
    BLOCK_TYPE selectedBlock;
    float camSpeed, exposure;
    bool collisionsOn;
};

namespace config
{
    const int32_t RENDER_DISTANCE = 10;
    const int32_t LOAD_DISTANCE = RENDER_DISTANCE * 1.5;

    const int32_t MAX_LOADS_PER_FRAME = 32;
    const int32_t MAX_UNLOADS_PER_FRAME = MAX_LOADS_PER_FRAME;
    const int32_t THREAD_COUNT = std::thread::hardware_concurrency();
    const int32_t MAX_BAKES_PER_FRAME = THREAD_COUNT - 1;

    const int32_t WORLD_SEED = 42;

    const float REACH_DISTANCE = 16.0f;
}
