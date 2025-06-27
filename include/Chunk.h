#pragma once

#include "Block.h"
#include "VertexArray.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "FastNoiseLite.h"

struct Chunk
{
    Chunk();
    Chunk(const glm::uvec2& chunkPosition, const FastNoiseLite& noise);
    void bake();
    BLOCK_TYPE getBlockUnsafe(const glm::uvec3& pos) const;
    BLOCK_TYPE getBlockSafe(const glm::uvec3& pos) const;

    static constexpr uint32_t CHUNK_SIZE = 16;
    static constexpr uint32_t MAX_HEIGHT = 16;
    static constexpr uint32_t MIN_HEIGHT = MAX_HEIGHT / 2;
    static constexpr uint32_t BLOCKS_PER_CHUNK = CHUNK_SIZE * CHUNK_SIZE * MAX_HEIGHT;

    VertexArray vao;
    glm::uvec2 chunkPosition;
    BLOCK_TYPE blocks[BLOCKS_PER_CHUNK];
    bool isDirty = true;
};