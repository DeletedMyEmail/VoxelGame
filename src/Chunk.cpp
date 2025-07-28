#include "Chunk.h"
#include <algorithm>
#include <ranges>
#include "Camera.h"
#include "OpenGLHelper.h"
#include "Shader.h"
#include "glm/common.hpp"

// CHUNK MANAGER ---------------------------------------

constexpr uint32_t getChunkCount() { return config::LOAD_DISTANCE * config::LOAD_DISTANCE;}

ChunkManager::ChunkManager()
    : threadPool(config::LOADING_THREADS), chunksToLoad{}, noise(config::WORLD_SEED)
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
            chunk.isBaked = false;
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

            if (chunk->isBaked)
            {
                if (chunk->vao.vertexCount == 0)
                    continue;

                chunk->vao.bind();
                setUniform3f(shader, "u_chunkOffset", glm::vec3(chunkPosToWorldBlockPos(chunk->chunkPosition)));
                GLCall(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, chunk->vao.vertexCount));
            }
            else if (chunksBaked < config::MAX_BAKES_PER_FRAME)
            {
                // TODO: race condition ?
                //threadPool.queueJob([chunk, this]()
                //{
                auto pos = chunk->chunkPosition;
                Chunk* leftChunk = getChunk({pos.x - 1, pos.y});
                Chunk* rightChunk = getChunk({pos.x + 1, pos.y});
                Chunk* frontChunk = getChunk({pos.x, pos.y + 1});
                Chunk* backChunk = getChunk({pos.x, pos.y - 1});
                chunk->bake(leftChunk, rightChunk, frontChunk, backChunk);
                //});
                chunksBaked++;
            }
        }
    }

    while (threadPool.busy())
        std::this_thread::sleep_for(std::chrono::microseconds(1));
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
                    *chunk = Chunk(chunkPos, noise, config::WORLD_BIOME);
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
        chunk.isBaked = false;
}

// CHUNK ---------------------------------------

static uint32_t getBlockIndex(const glm::ivec3& pos) { return pos.x + pos.y * Chunk::CHUNK_SIZE + pos.z * Chunk::CHUNK_SIZE * Chunk::CHUNK_SIZE; }

uint32_t noiseToHeight(const float value)
{
    const float normalized = (value + 1.0f) * 0.5f;
    return Chunk::MIN_GEN_HEIGHT + normalized * (Chunk::MAX_GEN_HEIGHT - Chunk::MIN_GEN_HEIGHT);
}

Chunk::Chunk()
    : blocks{}, chunkPosition({0}), isBaked(false), isLoaded(false)
{
}

Chunk::Chunk(const glm::ivec2& chunkPosition, const FastNoiseLite& noise, const BIOME biome)
    : blocks{}, chunkPosition(chunkPosition), isBaked(false), isLoaded(true)
{
    const BLOCK_TYPE defaultBlock = defaultBiomeBlock(biome);
    for (uint32_t x = 0; x < CHUNK_SIZE; x++)
    {
        for (uint32_t z = 0; z < CHUNK_SIZE; z++)
        {
            const glm::ivec3 worldPos = chunkPosToWorldBlockPos(chunkPosition) + glm::ivec3{x, 0, z};
            const uint32_t localHeight = noiseToHeight(noise.GetNoise((float) worldPos.x, (float) worldPos.z));

            for (uint32_t y = 0; y < CHUNK_SIZE; y++)
            {
                const auto index = getBlockIndex({x,y,z});
                if (y >= localHeight)
                    blocks[index] = BLOCK_TYPE::AIR;
                else
                    blocks[index] = y > localHeight-5 ? defaultBlock : BLOCK_TYPE::STONE;
            }
        }
    }
}

void Chunk::bake(Chunk* leftChunk, Chunk* rightChunk, Chunk* frontChunk, Chunk* backChunk)
{
    std::vector<blockdata> buffer;
    buffer.reserve(BLOCKS_PER_CHUNK / 2);
    uint32_t faceCount = 0;

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

                for (uint32_t i = 0; i < 6; i++)
                {
                    if (i == BOTTOM && y == 0)
                        continue;
                    if (i == TOP && y == CHUNK_SIZE - 1)
                        continue;

                    glm::ivec3 neighbourBlockPos;
                    switch (i)
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

                    auto atlasOffset = getAtlasOffset(block, FACE(i));
                    LOG_INFO("{} {} ", atlasOffset.x, atlasOffset.y);
                    blockdata packedData = ((i & 0xF) << 28) |
                                          ((x & 0x1F) << 23) |
                                          ((y & 0x1F) << 18) |
                                          ((z & 0x1F) << 13) |
                                          ((atlasOffset.x & 0xF) << 9) |
                                          ((atlasOffset.y & 0xF) << 5);
                    buffer.push_back(packedData);
                    faceCount++;
                }
            }
        }
    }
    
    vao.clear();
    const GLuint instanceVbo = createBuffer(buffer.data(), buffer.size() * sizeof(blockdata));
    VertexBufferLayout layout;
    layout.pushUInt(1, false, 1);
    vao.addBuffer(instanceVbo, layout);
    vao.vertexCount = faceCount;
    isBaked = true;
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
    isBaked = false;
}

void Chunk::setBlockSafe(const glm::ivec3& pos, BLOCK_TYPE block)
{
    if (inBounds(pos))
        setBlockUnsafe(pos, block);
    else
        LOG_WARN("BLock not found in chunk: ({}, {}, {})", pos.x, pos.y, pos.z);
}