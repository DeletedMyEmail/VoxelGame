#pragma once

#include "Block.h"
#include "VertexArray.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
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

struct Chunk
{
    Chunk();
    Chunk(const glm::uvec2& chunkPosition, const FastNoiseLite& noise, BIOME biome);
    void bake();
    BLOCK_TYPE getBlockUnsafe(const glm::uvec3& pos) const;
    BLOCK_TYPE getBlockSafe(const glm::uvec3& pos) const;

    static constexpr uint32_t CHUNK_SIZE = 16;
    static constexpr uint32_t MAX_HEIGHT = 256;
    static constexpr uint32_t MAX_GEN_HEIGHT = 128;
    static constexpr uint32_t MIN_GEN_HEIGHT = 32;
    static constexpr uint32_t BLOCKS_PER_CHUNK = CHUNK_SIZE * CHUNK_SIZE * MAX_HEIGHT;

    VertexArray vao;
    glm::uvec2 chunkPosition;
    BLOCK_TYPE blocks[BLOCKS_PER_CHUNK];
    bool isDirty = true;
};

inline glm::vec3 chunkPosToWorldPos(const glm::uvec2& chunkPos) { return {chunkPos.x * Chunk::CHUNK_SIZE, 0.0f, chunkPos.y * Chunk::CHUNK_SIZE}; }
