#include "Chunk.h"
#include <algorithm>
#include "Camera.h"
#include "OpenGLHelper.h"
#include "Shader.h"
#include "WorldGeneration.h"
#include "glm/common.hpp"

// CHUNK MANAGER ---------------------------------------

constexpr uint32_t getChunkCount() { return config::LOAD_DISTANCE * config::LOAD_DISTANCE;}

ChunkManager::ChunkManager()
    : threadPool(config::THREAD_COUNT)
{
    chunks.resize((1 + 2 * config::LOAD_DISTANCE) * (1 + 2 * config::LOAD_DISTANCE) * (1 + WORLD_HEIGHT));
}

void ChunkManager::unloadChunks(const glm::ivec3& currChunkPos)
{
    uint32_t unloads = 0;
    for (auto& chunk : chunks)
    {
        if (!chunk.isLoaded)
            continue;

        if (glm::abs(chunk.chunkPosition.x - currChunkPos.x) > config::LOAD_DISTANCE ||
            glm::abs(chunk.chunkPosition.y - currChunkPos.y) > config::LOAD_DISTANCE ||
            glm::abs(chunk.chunkPosition.z - currChunkPos.z) > config::LOAD_DISTANCE)
        {
            chunk.isLoaded = false;
            chunk.isMeshBaked = false;
            chunk.isMeshDataReady = false;
            unloads++;
        }

        if (unloads >= config::MAX_UNLOADS_PER_FRAME)
            break;
    }
}

void ChunkManager::drawChunks(const GLuint shader, const glm::ivec3& currChunkPos)
{
    uint32_t chunksBaked = 0;
    auto chunkQueue = getChunksSorted(currChunkPos, config::RENDER_DISTANCE);
    while (!chunkQueue.empty())
    {
        auto [position, priority] = chunkQueue.top();
        chunkQueue.pop();

        Chunk* chunk = getLoadedChunk(position);
        if (chunk == nullptr || !chunk->isLoaded)
            continue;

        if (chunk->isMeshBaked)
        {
            if (chunk->vao.vertexCount == 0)
                continue;

            chunk->vao.bind();
            setUniform3f(shader, "u_chunkOffset", glm::vec3(chunkPosToWorldBlockPos(chunk->chunkPosition)));
            GLCall(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, chunk->vao.vertexCount));
        }
        else if (chunksBaked < config::MAX_BAKES_PER_FRAME)
        {
            chunksBaked++;
            threadPool.queueJob([this, chunk, position]()
            {
                Chunk* leftChunk = getLoadedChunk({position.x - 1, position.y, position.z});
                Chunk* rightChunk = getLoadedChunk({position.x + 1, position.y, position.z});
                Chunk* frontChunk = getLoadedChunk({position.x, position.y, position.z + 1});
                Chunk* backChunk = getLoadedChunk({position.x, position.y, position.z - 1});
                Chunk* topChunk = getLoadedChunk({position.x, position.y + 1, position.z});
                Chunk* bottomChunk = getLoadedChunk({position.x, position.y - 1, position.z});

                chunk->generateMeshData(leftChunk, rightChunk, frontChunk, backChunk, topChunk, bottomChunk);
            });
        }
    }

    while (threadPool.busy())
        std::this_thread::sleep_for(std::chrono::microseconds(1));

    for (auto& chunk : chunks)
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

        Chunk* chunk = getChunkOrUnloaded(position);
        assert(chunk != nullptr);
        if (chunk->isLoaded) continue;

        chunk->isLoaded = true;
        chunksLoaded++;
        threadPool.queueJob([chunk, position, this]()
        {
            *chunk = Chunk(position);
        });
    }

    while (threadPool.busy())
        std::this_thread::sleep_for(std::chrono::microseconds(1));
}

Chunk* ChunkManager::getLoadedChunk(const glm::ivec3& pos)
{
    for (auto& chunk : chunks)
        if (chunk.chunkPosition == pos)
        {
            if (chunk.isLoaded)
                return &chunk;
            return nullptr;
        }

    return nullptr;
}

void ChunkManager::dropChunkMeshes()
{
    for (auto& chunk : chunks)
    {
        chunk.isMeshBaked = false;
        chunk.isMeshDataReady = false;
    }
}

Chunk* ChunkManager::getChunkOrUnloaded(const glm::ivec3& chunkPos)
{
    Chunk* chunk = nullptr;
    for (auto& c : chunks)
    {
        if (c.chunkPosition == chunkPos)
        {
            chunk = &c;
            break;
        }
        if (!c.isLoaded)
            chunk = &c;
    }

    return chunk;
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
    : blocks{}, chunkPosition({0}), isLoaded(false), isMeshBaked(false), isMeshDataReady(false)
{
}

Chunk::Chunk(const glm::ivec3& chunkPosition)
    : blocks{}, chunkPosition(chunkPosition), isLoaded(true), isMeshBaked(false), isMeshDataReady(false)
{
    meshData.reserve(BLOCKS_PER_CHUNK / 2);
    for (uint32_t x = 0; x < CHUNK_SIZE; x++)
    {
        for (uint32_t z = 0; z < CHUNK_SIZE; z++)
        {
            const glm::ivec3 worldPos = chunkPosToWorldBlockPos(chunkPosition) + glm::ivec3{x, 0, z};
            const uint32_t localHeight = getHeightAt({worldPos.x, worldPos.z});
            const uint32_t chunkHeight = chunkPosition.y * CHUNK_SIZE;

            for (uint32_t y = 0; y < CHUNK_SIZE; y++)
            {
                const auto index = getBlockIndex({x,y,z});
                const uint32_t worldY = y + chunkHeight;

                if (worldY >= localHeight) // Above terrain height
                {
                    if (worldY <= SEA_LEVEL && localHeight < SEA_LEVEL)
                        blocks[index] = BLOCK_TYPE::WATER;
                    else
                        blocks[index] = BLOCK_TYPE::AIR;
                }
                else
                {
                    // Below terrain height - place terrain blocks
                    blocks[index] = worldY > localHeight - 3 ? BLOCK_TYPE::GRASS : BLOCK_TYPE::STONE;
                }
            }
        }
    }
}

void Chunk::generateMeshData(Chunk* leftChunk, Chunk* rightChunk, Chunk* frontChunk, Chunk* backChunk, Chunk* topChunk, Chunk* bottomChunk)
{
    meshData.clear();
    for (uint32_t z = 0; z < CHUNK_SIZE; z++)
    {
        for (uint32_t y = 0; y < CHUNK_SIZE; y++)
        {
            for (uint32_t x = 0; x < CHUNK_SIZE; x++)
            {
                const auto& block = getBlockUnsafe({x,y,z});

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
                    if (neighbourBlock != BLOCK_TYPE::INVALID && neighbourBlock != BLOCK_TYPE::AIR) // neighbour is solid
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
                            if (neighbourBlock != BLOCK_TYPE::AIR)
                                continue;
                        }
                    }

                    auto atlasOffset = getAtlasOffset(block, FACE(face));
                    blockdata packedData = ((face & FACE_MASK) << FACE_OFFSET) |
                                          ((x & XPOS_MASK) << XPOS_OFFSET) |
                                          ((y & YPOS_MASK) << YPOS_OFFSET) |
                                          ((z & ZPOS_MASK) << ZPOS_OFFSET) |
                                          ((atlasOffset.x & ATLASX_MASK) << ATLASX_OFFSET) |
                                          ((atlasOffset.y & ATLASY_MASK) << ATLASY_OFFSET);
                    meshData.push_back(packedData);
                }
            }
        }
    }
    isMeshDataReady = true;
    isMeshBaked = false;
}

void Chunk::bakeMesh()
{
    vao.clear();
    const GLuint instanceVbo = createBuffer(meshData.data(), meshData.size() * sizeof(blockdata));
    VertexBufferLayout layout;
    layout.pushUInt(1, false, 1);
    vao.addBuffer(instanceVbo, layout);
    vao.vertexCount = meshData.size();
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
