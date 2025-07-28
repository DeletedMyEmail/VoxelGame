#pragma once

#include <bitset>
#include <unordered_map>
#include "Block.h"
#include "Config.h"
#include "VertexArray.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "FastNoiseLite.h"
#include "ThreadPool.h"
#include "WorldGeneration.h"

struct Chunk;

struct ChunkManager
{
    ChunkManager();
    void unloadChunks(const glm::ivec2& currChunkPos);
    void drawChunks(GLuint shader, const glm::ivec2& currChunkPos);
    void loadChunks(const glm::ivec2& currChunkPos);
    Chunk* getChunk(glm::ivec2 pos);
    void dropChunkMeshes();

    ThreadPool threadPool;
    std::vector<Chunk> chunks;
    std::mutex chunksMutex;
    uint64_t chunksToLoad[config::MAX_LOADS_PER_FRAME];
    uint32_t numChunksToLoad = 0;
    FastNoiseLite noise;
};

struct Chunk
{
    Chunk();
    Chunk(const glm::ivec2& chunkPosition, const FastNoiseLite& noise, BIOME biome);;
    void bake(Chunk* leftChunk, Chunk* rightChunk, Chunk* frontChunk, Chunk* backChunk);
    BLOCK_TYPE getBlockUnsafe(const glm::ivec3& pos) const;
    BLOCK_TYPE getBlockSafe(const glm::ivec3& pos) const;
    void setBlockUnsafe(const glm::ivec3& pos, BLOCK_TYPE block);
    void setBlockSafe(const glm::ivec3& pos, BLOCK_TYPE block);

    static constexpr int32_t CHUNK_SIZE = 32;
    static constexpr int32_t MAX_GEN_HEIGHT = 32;
    static constexpr int32_t MIN_GEN_HEIGHT = 4;
    static constexpr int32_t BLOCKS_PER_CHUNK = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

    BLOCK_TYPE blocks[BLOCKS_PER_CHUNK];
    VertexArray vao;
    glm::ivec2 chunkPosition;
    //std::bitset<CHUNK_SIZE> bitmap[CHUNK_SIZE][CHUNK_SIZE];
    bool isBaked;
    bool isLoaded;
};

inline glm::ivec3 chunkPosToWorldBlockPos(const glm::ivec2& chunkPos) { return glm::ivec3{chunkPos.x * Chunk::CHUNK_SIZE, 0.0f, chunkPos.y * Chunk::CHUNK_SIZE}; }
inline glm::ivec3 worldPosToChunkBlockPos(const glm::ivec3& worldPos) { return glm::ivec3{worldPos.x % Chunk::CHUNK_SIZE, worldPos.y,worldPos.z % Chunk::CHUNK_SIZE};}
inline glm::ivec2 worldPosToChunkPos(const glm::ivec3& worldPos) { return glm::ivec2{worldPos.x / Chunk::CHUNK_SIZE,worldPos.z / Chunk::CHUNK_SIZE}; }
inline bool inBounds(const glm::ivec3& pos) { return pos.x < Chunk::CHUNK_SIZE && pos.x >= 0 && pos.y < Chunk::CHUNK_SIZE && pos.y >= 0 && pos.z < Chunk::CHUNK_SIZE && pos.z >= 0; }
