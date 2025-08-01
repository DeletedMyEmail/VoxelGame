#include "Chunk.h"
#include <algorithm>
#include "Camera.h"
#include "OpenGLHelper.h"
#include "Rendering.h"
#include "Shader.h"
#include "WorldGeneration.h"
#include "glm/common.hpp"

// CHUNK MANAGER ---------------------------------------

constexpr uint32_t getChunkCount() { return config::LOAD_DISTANCE * config::LOAD_DISTANCE;}

ChunkManager::ChunkManager()
    : threadPool(config::THREAD_COUNT)
{
    chunks.reserve((2 * config::LOAD_DISTANCE) * (2 * config::LOAD_DISTANCE) * (WORLD_HEIGHT));
}

void ChunkManager::unloadChunks(const glm::ivec3& currChunkPos)
{
    uint32_t unloads = 0;
    for (auto it = chunks.begin(); it != chunks.end() && unloads < config::MAX_UNLOADS_PER_FRAME;)
    {
        const Chunk& chunk = it->second;

        if (glm::abs(chunk.chunkPosition.x - currChunkPos.x) > config::LOAD_DISTANCE ||
            glm::abs(chunk.chunkPosition.y - currChunkPos.y) > config::LOAD_DISTANCE ||
            glm::abs(chunk.chunkPosition.z - currChunkPos.z) > config::LOAD_DISTANCE)
        {
            it = chunks.erase(it);
            unloads++;
        }
        else
            ++it;
    }
}

void ChunkManager::drawChunks(const glm::mat4& viewProjection, const float exposure) const
{
    for (auto& [_, chunk] : chunks)
    {
        if (chunk.isMeshBaked)
            drawChunk(chunk.vaoOpaque, chunkPosToWorldBlockPos(chunk.chunkPosition), viewProjection, exposure);
    }

    for (auto& [_, chunk] : chunks)
    {
        if (chunk.isMeshBaked)
            drawChunk(chunk.vaoTranslucent, chunkPosToWorldBlockPos(chunk.chunkPosition), viewProjection, exposure);
    }
}

void ChunkManager::bakeChunks(const glm::ivec3& currChunkPos)
{
    uint32_t chunksBaked = 0;
    auto chunkQueue = getChunksSorted(currChunkPos, config::RENDER_DISTANCE);

    while (!chunkQueue.empty())
    {
        auto [position, priority] = chunkQueue.top();
        chunkQueue.pop();

        auto it = chunks.find(position);
        if (it == chunks.end())
            continue;

        Chunk& chunk = it->second;
        if (chunk.isMeshBaked || chunksBaked >= config::MAX_BAKES_PER_FRAME)
            continue;

        chunksBaked++;
        threadPool.queueJob([this, &chunk, position]()
        {
            Chunk* leftChunk = getChunk({position.x - 1, position.y, position.z});
            Chunk* rightChunk = getChunk({position.x + 1, position.y, position.z});
            Chunk* frontChunk = getChunk({position.x, position.y, position.z + 1});
            Chunk* backChunk = getChunk({position.x, position.y, position.z - 1});
            Chunk* topChunk = getChunk({position.x, position.y + 1, position.z});
            Chunk* bottomChunk = getChunk({position.x, position.y - 1, position.z});

            chunk.generateMeshData(leftChunk, rightChunk, frontChunk, backChunk, topChunk, bottomChunk);
        });
    }

    while (threadPool.busy())
        std::this_thread::sleep_for(std::chrono::microseconds(1));

    for (auto& [_, chunk] : chunks)
    {
        if (chunk.isMeshDataReady && !chunk.isMeshBaked)
            chunk.bakeMesh();
    }
}

void ChunkManager::loadChunks(const glm::ivec3& currChunkPos)
{
    auto chunkQueue = getChunksSorted(currChunkPos, config::LOAD_DISTANCE);

    uint32_t chunksLoaded = 0;
    while (!chunkQueue.empty() && chunksLoaded < config::MAX_LOADS_PER_FRAME)
    {
        auto [position, priority] = chunkQueue.top();
        chunkQueue.pop();

        if (chunks.contains(position))
            continue;

        chunksLoaded++;
        Chunk* chunk = &chunks.emplace(position, Chunk()).first->second;
        threadPool.queueJob([chunk, position, this]()
        {
            *chunk = Chunk(position);
        });
    }

    while (threadPool.busy())
        std::this_thread::sleep_for(std::chrono::microseconds(1));
}

void ChunkManager::dropChunkMeshes()
{
    for (auto& [_, chunk] : chunks)
    {
        chunk.isMeshBaked = false;
        chunk.isMeshDataReady = false;
    }
}

Chunk* ChunkManager::getChunk(const glm::ivec3& pos)
{
    const auto it = chunks.find(pos);
    if (it != chunks.end())
        return &it->second;
    return nullptr;
}


std::priority_queue<ChunkManager::ChunkLoadRequest> ChunkManager::getChunksSorted(const glm::ivec3& currChunkPos, const int32_t maxDist)
{
    uint32_t chunksAdded = 0;
    std::priority_queue<ChunkLoadRequest> queue;
    for (int32_t x = currChunkPos.x - maxDist; x <= currChunkPos.x + maxDist; x++)
    {
        for (int32_t y = 0; y < WORLD_HEIGHT; y++)
        {
            for (int32_t z = currChunkPos.z - maxDist; z <= currChunkPos.z + maxDist; z++)
            {
                glm::ivec3 chunkPos = {x, y, z};
                const float distance = glm::length(glm::vec3(chunkPos - currChunkPos));
                queue.push({chunkPos, distance});
                chunksAdded++;
            }
        }
    }

    return queue;
}

// CHUNK ---------------------------------------

static uint32_t getBlockIndex(const glm::ivec3& pos) { return pos.x + pos.y * Chunk::CHUNK_SIZE + pos.z * Chunk::CHUNK_SIZE * Chunk::CHUNK_SIZE; }

Chunk::Chunk()
    : blocks{}, chunkPosition({0}), isMeshBaked(false), isMeshDataReady(false)
{
}

Chunk::Chunk(const glm::ivec3& chunkPosition)
    : blocks{}, chunkPosition(chunkPosition), isMeshBaked(false), isMeshDataReady(false)
{
    meshDataOpaque.reserve(BLOCKS_PER_CHUNK / 2);
    meshDataTranslucent.reserve(BLOCKS_PER_CHUNK / 2);

    const auto absChunkPos = chunkPosToWorldBlockPos(chunkPosition);
    const uint32_t chunkHeight = chunkPosition.y * CHUNK_SIZE;

    for (uint32_t x = 0; x < CHUNK_SIZE; x++)
    {
        for (uint32_t z = 0; z < CHUNK_SIZE; z++)
        {
            glm::ivec3 absPos = absChunkPos + glm::ivec3{x, 0, z};
            const uint32_t terrainHeight = getHeightAt({absPos.x, absPos.z});
            const bool tree = hasTree({absPos.x, absPos.z});

            for (uint32_t y = 0; y < CHUNK_SIZE; y++)
            {
                const uint32_t index = getBlockIndex({x,y,z});
                const uint32_t absY = y + chunkHeight;

                if (absY >= terrainHeight)
                {
                    if (absY <= SEA_LEVEL && terrainHeight <= SEA_LEVEL)
                        blocks[index] = BLOCK_TYPE::WATER;
                    else
                        blocks[index] = BLOCK_TYPE::AIR;
                }
                else
                    blocks[index] = absY > terrainHeight - 3 ? BLOCK_TYPE::GRASS : BLOCK_TYPE::STONE;

                if (tree && terrainHeight > SEA_LEVEL && absY > SEA_LEVEL && absY >= terrainHeight && absY < terrainHeight + 6)
                    blocks[index] = BLOCK_TYPE::WOOD;
            }
        }
    }
}

void Chunk::generateMeshData(Chunk* leftChunk, Chunk* rightChunk, Chunk* frontChunk, Chunk* backChunk, Chunk* topChunk, Chunk* bottomChunk)
{
    meshDataOpaque.clear();
    meshDataTranslucent.clear();

    for (uint32_t z = 0; z < CHUNK_SIZE; z++)
    {
        for (uint32_t y = 0; y < CHUNK_SIZE; y++)
        {
            for (uint32_t x = 0; x < CHUNK_SIZE; x++)
            {
                glm::uvec3 blockPos = {x, y, z};
                const auto& block = getBlockUnsafe(blockPos);

                assert(block != BLOCK_TYPE::INVALID);
                if (block == BLOCK_TYPE::AIR)
                    continue;

                for (uint32_t face = 0; face < 6; face++)
                {
                    if (face == BOTTOM && y == 0)
                        continue;

                    glm::ivec3 neighbourBlockPos;
                    switch (face)
                    {
                        case BACK: neighbourBlockPos = {x, y, z - 1}; break;
                        case FRONT: neighbourBlockPos = {x, y, z + 1}; break;
                        case LEFT: neighbourBlockPos = {x - 1, y, z}; break;
                        case RIGHT: neighbourBlockPos = {x + 1, y, z}; break;
                        case TOP: neighbourBlockPos = {x, y + 1, z}; break;
                        case BOTTOM: neighbourBlockPos = {x, y - 1, z}; break;
                        default: assert(false);
                    }

                    BLOCK_TYPE neighbourBlock = getBlockSafe(neighbourBlockPos);
                    if (neighbourBlock != BLOCK_TYPE::INVALID && neighbourBlock != BLOCK_TYPE::AIR && !(block != BLOCK_TYPE::WATER && neighbourBlock == BLOCK_TYPE::WATER))
                        continue;

                    if (neighbourBlock == BLOCK_TYPE::INVALID) // neighbour in different chunk
                    {
                        // check if a neighboring chunk has a covering block
                        glm::ivec3 blockPosInOtherChunk = neighbourBlockPos;
                        Chunk* neighborChunk = nullptr;

                        if (neighbourBlockPos.x == CHUNK_SIZE)
                        {
                            neighborChunk = rightChunk;
                            blockPosInOtherChunk.x = 0;
                        }
                        else if (neighbourBlockPos.x == -1)
                        {
                            neighborChunk = leftChunk;
                            blockPosInOtherChunk.x = CHUNK_SIZE - 1;
                        }
                        else if (neighbourBlockPos.y == CHUNK_SIZE)
                        {
                            neighborChunk = topChunk;
                            blockPosInOtherChunk.y = 0;
                        }
                        else if (neighbourBlockPos.y == -1)
                        {
                            neighborChunk = bottomChunk;
                            blockPosInOtherChunk.y = CHUNK_SIZE - 1;
                        }
                        else if (neighbourBlockPos.z == CHUNK_SIZE)
                        {
                            neighborChunk = frontChunk;
                            blockPosInOtherChunk.z = 0;
                        }
                        else if (neighbourBlockPos.z == -1)
                        {
                            neighborChunk = backChunk;
                            blockPosInOtherChunk.z = CHUNK_SIZE - 1;
                        }

                        if (neighborChunk)
                        {
                            neighbourBlock = neighborChunk->getBlockUnsafe(blockPosInOtherChunk);
                            if (neighbourBlock != BLOCK_TYPE::AIR && !(block != BLOCK_TYPE::WATER && neighbourBlock == BLOCK_TYPE::WATER))
                                continue;
                        }
                    }

                    auto atlasOffset = getAtlasOffset(block, FACE(face));
                    if (block == BLOCK_TYPE::WATER)
                        meshDataTranslucent.push_back(packBlockData(blockPos, atlasOffset, FACE(face)));
                    else
                        meshDataOpaque.push_back(packBlockData(blockPos, atlasOffset, FACE(face)));
                }
            }
        }
    }

    isMeshDataReady = true;
    isMeshBaked = false;
}

void bake(VertexArray& vao, const std::vector<blockdata>& meshData)
{
    VertexBufferLayout layout;
    layout.pushUInt(1, false, 1);

    const GLuint instanceVbo = createBuffer(meshData.data(), meshData.size() * sizeof(blockdata));

    vao.clear();
    vao.addBuffer(instanceVbo, layout);
    vao.vertexCount = meshData.size() * 6;
}

void Chunk::bakeMesh()
{
    bake(vaoOpaque, meshDataOpaque);
    bake(vaoTranslucent, meshDataTranslucent);
    isMeshBaked = true;
}

BLOCK_TYPE Chunk::getBlockUnsafe(const glm::ivec3& pos) const
{
    return blocks[getBlockIndex(pos)];
}

BLOCK_TYPE Chunk::getBlockSafe(const glm::ivec3& pos) const
{
    if (isChunkCoord(pos))
        return getBlockUnsafe(pos);

    return BLOCK_TYPE::INVALID;
}

void Chunk::setBlockUnsafe(const glm::ivec3& pos, const BLOCK_TYPE block)
{
    blocks[getBlockIndex(pos)] = block;
    isMeshBaked = false;
    isMeshDataReady = false;
}

void Chunk::setBlockSafe(const glm::ivec3& pos, const BLOCK_TYPE block)
{
    if (isChunkCoord(pos))
        setBlockUnsafe(pos, block);
    else
        LOG_WARN("BLock not found in chunk: ({}, {}, {})", pos.x, pos.y, pos.z);
}

glm::ivec3 chunkPosToWorldBlockPos(const glm::ivec3& chunkPos)
{
    return chunkPos * Chunk::CHUNK_SIZE;
}

glm::ivec3 worldPosToChunkBlockPos(const glm::ivec3& worldPos)
{
    return worldPos % Chunk::CHUNK_SIZE;
}

glm::ivec3 worldPosToChunkPos(const glm::ivec3& worldPos)
{
    return worldPos / Chunk::CHUNK_SIZE;
}

bool isChunkCoord(const glm::ivec3& pos)
{
    return pos.x < Chunk::CHUNK_SIZE && pos.x >= 0 &&
           pos.y < Chunk::CHUNK_SIZE && pos.y >= 0 &&
           pos.z < Chunk::CHUNK_SIZE && pos.z >= 0;
}
