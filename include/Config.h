#pragma once
#include <thread>

#include "WorldGeneration.h"


namespace config
{
    const uint32_t RENDER_DISTANCE = 32;
    const uint32_t LOAD_DISTANCE = RENDER_DISTANCE * 2;

    const uint32_t MAX_LOADS_PER_FRAME = 32;
    const uint32_t MAX_UNLOADS_PER_FRAME = 32;
    const uint32_t LOADING_THREADS = 32;
    const uint32_t MAX_BAKES_PER_FRAME = 32;

    const BIOME WORLD_BIOME = HILLS;
    const uint32_t WORLD_SEED = 1337;
    //const uint32_t WORLD_CHUNK_SIZE = 128;
    //const glm::uvec3 WORLD_SIZE{WORLD_CHUNK_SIZE * Chunk::CHUNK_SIZE, Chunk::MAX_HEIGHT, WORLD_CHUNK_SIZE * Chunk::CHUNK_SIZE};

    const float REACH_DISTANCE = 16.0f;
}
