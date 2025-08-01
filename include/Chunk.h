#pragma once

#include "Block.h"
#include "Config.h"
#include "VertexArray.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "ThreadPool.h"
#include "glm/fwd.hpp"

struct Chunk
{
    Chunk();
    Chunk(const glm::ivec3& chunkPosition);;
    void generateMeshData(Chunk* leftChunk, Chunk* rightChunk, Chunk* frontChunk, Chunk* backChunk, Chunk* topChunk, Chunk* bottomChunk);
    void bakeMesh();
    BLOCK_TYPE getBlockUnsafe(const glm::ivec3& pos) const;
    BLOCK_TYPE getBlockSafe(const glm::ivec3& pos) const;
    void setBlockUnsafe(const glm::ivec3& pos, BLOCK_TYPE block);
    void setBlockSafe(const glm::ivec3& pos, BLOCK_TYPE block);

    static constexpr int32_t CHUNK_SIZE = 32;
    static constexpr int32_t BLOCKS_PER_CHUNK = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

    BLOCK_TYPE blocks[BLOCKS_PER_CHUNK];
    std::vector<blockdata> meshDataOpaque, meshDataTranslucent;
    VertexArray vaoOpaque, vaoTranslucent;
    glm::ivec3 chunkPosition;
    bool isMeshBaked, isMeshDataReady;
};

glm::ivec3 chunkPosToWorldBlockPos(const glm::ivec3& chunkPos);
glm::ivec3 worldPosToChunkBlockPos(const glm::ivec3& worldPos);
glm::ivec3 worldPosToChunkPos(const glm::ivec3& worldPos);
bool isChunkCoord(const glm::ivec3& pos);

template<>
struct std::hash<glm::ivec3>
{
    size_t operator()(const glm::ivec3& v) const noexcept
    {
        const size_t h1 = hash<int>{}(v.x);
        const size_t h2 = hash<int>{}(v.y);
        const size_t h3 = hash<int>{}(v.z);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

struct ChunkManager
{
    ChunkManager();
    void unloadChunks(const glm::ivec3& currChunkPos);
    void drawChunks(const glm::mat4& viewProjection, float exposure) const;
    void bakeChunks(const glm::ivec3& currChunkPos);
    void loadChunks(const glm::ivec3& currChunkPos);
    void dropChunkMeshes();
    Chunk* getChunk(const glm::ivec3& pos);

    struct ChunkLoadRequest
    {
        glm::ivec3 position;
        float priority;

        bool operator<(const ChunkLoadRequest& other) const { return priority > other.priority; }
    };

    static std::priority_queue<ChunkLoadRequest> getChunksSorted(const glm::ivec3& currChunkPos, int32_t maxDist);

    ThreadPool threadPool;
    std::unordered_map<glm::ivec3, Chunk> chunks;
};