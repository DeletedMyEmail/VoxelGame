#pragma once
#include "Chunk.h"

namespace config
{
    const float RENDER_DISTANCE = Chunk::CHUNK_SIZE * 32;
    const uint32_t MAX_BAKES_PER_FRAME = 4;

    const BIOME WORLD_BIOME = HILLS;
    uint32_t WORLD_CHUNK_SIZE = 128;
    const glm::uvec3 WORLD_SIZE{WORLD_CHUNK_SIZE * Chunk::CHUNK_SIZE, Chunk::MAX_HEIGHT, WORLD_CHUNK_SIZE * Chunk::CHUNK_SIZE};

    const float REACH_DISTANCE = 16.0f;
}
