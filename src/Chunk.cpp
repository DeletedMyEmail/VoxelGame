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
    : threadPool(config::THREAD_COUNT), chunksToLoad{}
{
    chunks.resize((1 + 2 * config::LOAD_DISTANCE) * (1 + 2 * config::LOAD_DISTANCE));
}

void ChunkManager::unloadChunks(const glm::ivec2& currChunkPos)
{
    uint32_t unloads = 0;
    for (auto& chunk : chunks)
    {
        if (!chunk.isLoaded)
            continue;

        if (glm::abs(chunk.chunkPosition.x - currChunkPos.x) > config::LOAD_DISTANCE ||
            glm::abs(chunk.chunkPosition.y - currChunkPos.y) > config::LOAD_DISTANCE)
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

void ChunkManager::drawChunks(const GLuint shader, const glm::ivec2& currChunkPos)
{
    uint32_t chunksBaked = 0;

    const int32_t maxRenderX = currChunkPos.x + config::RENDER_DISTANCE;
    const int32_t minRenderX = currChunkPos.x - config::RENDER_DISTANCE;

    const int32_t minRenderZ = currChunkPos.y - config::RENDER_DISTANCE;
    const int32_t maxRenderZ = currChunkPos.y + config::RENDER_DISTANCE;

    for (int32_t x = minRenderX ; x <= maxRenderX; x++)
    {
        for (int32_t z = minRenderZ ; z <= maxRenderZ; z++)
        {
            const glm::ivec2 chunkPos = {x, z};
            Chunk* chunk = getChunk(chunkPos);
            if (chunk == nullptr)
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
                ++chunksBaked;
                threadPool.queueJob([this, chunk, chunkPos]()
                {
                    chunk->generateMeshData(getChunk({chunkPos.x - 1, chunkPos.y}),
                        getChunk({chunkPos.x + 1, chunkPos.y}),
                        getChunk({chunkPos.x, chunkPos.y + 1}),
                        getChunk({chunkPos.x, chunkPos.y - 1}));
                });
            }
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

void ChunkManager::loadChunks(const glm::ivec2& currChunkPos)
{
    uint32_t chunksLoaded = 0;

    const int32_t maxLoadX = currChunkPos.x + config::LOAD_DISTANCE;
    const int32_t minLoadX = currChunkPos.x - config::LOAD_DISTANCE;

    const int32_t maxLoadZ = currChunkPos.y + config::LOAD_DISTANCE;
    const int32_t minLoadZ = currChunkPos.y - config::LOAD_DISTANCE;

    for (int32_t x = minLoadX ; x <= maxLoadX; x++)
    {
        for (int32_t z = minLoadZ ; z <= maxLoadZ; z++)
        {
            if (chunksLoaded >= config::MAX_LOADS_PER_FRAME)
                break;

            const glm::ivec2 chunkPos = {x, z};
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

            assert(chunk != nullptr);

            if (!chunk->isLoaded)
            {
                chunk->isLoaded = true;
                chunksLoaded++;
                threadPool.queueJob([chunk, chunkPos, this]()
                {
                    *chunk = Chunk(chunkPos);
                });
            }
        }
    }

    while (threadPool.busy())
        std::this_thread::sleep_for(std::chrono::microseconds(1));
}

Chunk* ChunkManager::getChunk(const glm::ivec2 pos)
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

// CHUNK ---------------------------------------

static uint32_t getBlockIndex(const glm::ivec3& pos) { return pos.x + pos.y * Chunk::CHUNK_SIZE + pos.z * Chunk::CHUNK_SIZE * Chunk::CHUNK_SIZE; }

Chunk::Chunk()
    : blocks{}, chunkPosition({0}), isLoaded(false), isMeshBaked(false), isMeshDataReady(false)
{
}

Chunk::Chunk(const glm::ivec2& chunkPosition)
    : blocks{}, chunkPosition(chunkPosition), isLoaded(true), isMeshBaked(false), isMeshDataReady(false)
{
    meshData.reserve(BLOCKS_PER_CHUNK / 2);
    for (uint32_t x = 0; x < CHUNK_SIZE; x++)
    {
        for (uint32_t z = 0; z < CHUNK_SIZE; z++)
        {
            const glm::ivec3 worldPos = chunkPosToWorldBlockPos(chunkPosition) + glm::ivec3{x, 0, z};
            const uint32_t localHeight = getHeightAt({worldPos.x, worldPos.z});

            for (uint32_t y = 0; y < CHUNK_SIZE; y++)
            {
                const auto index = getBlockIndex({x,y,z});
                if (y >= localHeight)
                    blocks[index] = BLOCK_TYPE::AIR;
                else
                    blocks[index] = y > localHeight-5 ? BLOCK_TYPE::GRASS : BLOCK_TYPE::STONE;
            }
        }
    }
}

static constexpr uint32_t
    FACE_MASK = 0xFu, FACE_OFFSET = 28u,
    XPOS_MASK = 0x1Fu, XPOS_OFFSET = 23u,
    YPOS_MASK = 0x1Fu, YPOS_OFFSET = 18u,
    ZPOS_MASK = 0x1Fu, ZPOS_OFFSET = 13u,
    ATLASX_MASK = 0xFu, ATLASX_OFFSET = 9u,
    ATLASY_MASK = 0xFu, ATLASY_OFFSET = 5u;

void Chunk::generateMeshData(Chunk* leftChunk, Chunk* rightChunk, Chunk* frontChunk, Chunk* backChunk)
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

                    const BLOCK_TYPE neighbourBlock = getBlockSafe(neighbourBlockPos);
                    if (neighbourBlock != BLOCK_TYPE::AIR && neighbourBlock != BLOCK_TYPE::INVALID)
                        continue;
                    if (neighbourBlock == BLOCK_TYPE::INVALID && neighbourBlockPos.y < CHUNK_SIZE)
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

                        if (neighbourBlockPos.z == CHUNK_SIZE)
                        {
                            neighborChunk = frontChunk;
                            blockPosInOtherChunk.z = 0;
                        }
                        else if (neighbourBlockPos.z == -1)
                        {
                            neighborChunk = backChunk;
                            blockPosInOtherChunk.z = CHUNK_SIZE - 1;
                        }

                        assert(!neighborChunk || neighborChunk->getBlockSafe(blockPosInOtherChunk) != BLOCK_TYPE::INVALID);
                        if (neighborChunk && neighborChunk->getBlockUnsafe(blockPosInOtherChunk) != BLOCK_TYPE::AIR)
                            continue;
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
    if (inBounds(pos))
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
    if (inBounds(pos))
        setBlockUnsafe(pos, block);
    else
        LOG_WARN("BLock not found in chunk: ({}, {}, {})", pos.x, pos.y, pos.z);
}