#pragma once

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
    void unloadChunks(const glm::uvec2& currChunkPos);
    void drawChunks(GLuint shader, const glm::vec3& cameraPosition);
    void loadChunks();
    Chunk* getChunk(glm::uvec2 pos);
    void dropChunkMeshes();
    uint32_t getChunkCount() const;

    ThreadPool threadPool;
    std::unordered_map<uint64_t, Chunk*> chunks;
    std::mutex chunksMutex;
    uint64_t chunksToLoad[config::MAX_LOADS_PER_FRAME];
    uint32_t numChunksToLoad = 0;
    FastNoiseLite noise;
};

struct Chunk
{
    Chunk();
    Chunk(const glm::uvec2& chunkPosition, const FastNoiseLite& noise, BIOME biome);;
    void bake(ChunkManager& chunkManager);
    BLOCK_TYPE getBlockUnsafe(const glm::uvec3& pos) const;
    BLOCK_TYPE getBlockSafe(const glm::uvec3& pos) const;
    void setBlockUnsafe(const glm::uvec3& pos, BLOCK_TYPE block);
    void setBlockSafe(const glm::uvec3& pos, BLOCK_TYPE block);

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

inline glm::uvec3 chunkPosToWorldBlockPos(const glm::uvec2& chunkPos) { return glm::uvec3{chunkPos.x * Chunk::CHUNK_SIZE, 0.0f, chunkPos.y * Chunk::CHUNK_SIZE}; }
inline glm::uvec3 worldPosToChunkBlockPos(const glm::uvec3& worldPos) { return glm::uvec3{worldPos.x % Chunk::CHUNK_SIZE, worldPos.y,worldPos.z % Chunk::CHUNK_SIZE};}
inline glm::uvec2 worldPosToChunkPos(const glm::uvec3& worldPos) { return glm::uvec2{worldPos.x / Chunk::CHUNK_SIZE,worldPos.z / Chunk::CHUNK_SIZE}; }
inline bool inBounds(const glm::uvec3& pos) { return pos.x < Chunk::CHUNK_SIZE && pos.y < Chunk::MAX_HEIGHT && pos.z < Chunk::CHUNK_SIZE; }
