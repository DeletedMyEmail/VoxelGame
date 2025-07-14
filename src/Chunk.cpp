#include "Chunk.h"

#include <latch>

#include "OpenGLHelper.h"
#include "Shader.h"
#include "cstmlib/Profiling.h"
#include "glm/common.hpp"

// CHUNK MANAGER ---------------------------------------

ChunkManager::ChunkManager()
    : threadPool(config::LOADING_THREADS), chunks(config::RENDER_DISTANCE * config::RENDER_DISTANCE), chunksToLoad{}, noise(config::WORLD_SEED)
{
}

void ChunkManager::unloadChunks(const glm::uvec2& currChunkPos)
{
    uint32_t unloads = 0;
    for (auto it = chunks.begin(); it != chunks.end() && unloads < config::MAX_UNLOADS_PER_FRAME;)
    {
        const Chunk* chunk = it->second;
        assert(chunk != nullptr && "Chunk pointer should not be null");

        const glm::ivec2 dist = glm::abs(glm::ivec2(chunk->chunkPosition) - glm::ivec2(currChunkPos));
        if (dist.x > config::LOAD_DISTANCE || dist.y > config::LOAD_DISTANCE)
        {
            delete chunk;
            it = chunks.erase(it);
            unloads++;
        }
        else
            ++it;
    }
}

static uint64_t packChunkPos(const glm::uvec2& chunkPos) { return (uint64_t(chunkPos.x) << 32) | chunkPos.y; }
static glm::uvec2 unpackChunkPos(const uint64_t packedPos) { return {packedPos >> 32, packedPos & 0xFFFF'FFFF}; }

void ChunkManager::drawChunks(const GLuint shader, const glm::vec3& cameraPosition)
{
    uint32_t chunksBaked = 0;
    numChunksToLoad = 0;

    const glm::uvec2 currChunkPos = worldPosToChunkPos(cameraPosition);

    uint32_t maxLoadX = currChunkPos.x + config::LOAD_DISTANCE;
    uint32_t minLoadX = currChunkPos.x - config::LOAD_DISTANCE;
    if (minLoadX > maxLoadX)
        minLoadX = 0;

    uint32_t minLoadZ = currChunkPos.y - config::LOAD_DISTANCE;
    uint32_t maxLoadZ = currChunkPos.y + config::LOAD_DISTANCE;
    if (minLoadZ > maxLoadZ)
        minLoadZ = 0;

    for (uint32_t x = minLoadX ; x < maxLoadX; x++)
    {
        for (uint32_t z = minLoadZ ; z < maxLoadZ; z++)
        {
            // add chunk for load
            glm::uvec2 chunkPos = {x, z};
            Chunk* chunk = getChunk(chunkPos);
            if (chunk == nullptr)
            {
                if (numChunksToLoad < config::MAX_LOADS_PER_FRAME)
                    chunksToLoad[numChunksToLoad++] = packChunkPos(chunkPos);
                continue;
            }

            if (chunk->chunkPosition != chunkPos)
            {
                LOG_WARN("key {} {}, val {} {}, cam {} {}", chunkPos.x, chunkPos.y, chunk->chunkPosition.x, chunk->chunkPosition.y, currChunkPos.x, currChunkPos.y);
                assert(chunk->chunkPosition == chunkPos);
            }

            // draw chunk
            uint32_t minRenderX = currChunkPos.x - config::RENDER_DISTANCE;
            uint32_t maxRenderX = currChunkPos.x + config::RENDER_DISTANCE;
            if (minRenderX > maxRenderX)
                minRenderX = 0;
            uint32_t minRenderZ = currChunkPos.y - config::RENDER_DISTANCE;
            uint32_t maxRenderZ = currChunkPos.y + config::RENDER_DISTANCE;
            if (minRenderZ > maxRenderZ)
                minRenderZ = 0;

            if (x < minRenderX || x > maxRenderX || z < minRenderZ || z > maxRenderZ)
                continue;

            if (chunk->isDirty)
            {
                if (chunksBaked >= config::MAX_BAKES_PER_FRAME)
                    continue;

                chunk->bake(*this);
                chunksBaked++;
            }

            chunk->vao.bind();
            setUniform3f(shader, "u_chunkOffset", chunkPosToWorldBlockPos(chunk->chunkPosition));
            GLCall(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, chunk->vao.vertexCount));
        }
    }
}

void loadWorker(uint64_t chunkPositions[], const uint32_t numChunks, ChunkManager& chunkManager)
{
    const FastNoiseLite localNoise = createBiomeNoise(config::WORLD_BIOME, config::WORLD_SEED);
    Chunk* newChunks[numChunks];

    for (uint32_t i = 0; i < numChunks; i++)
        newChunks[i] = new Chunk(unpackChunkPos(chunkPositions[i]), localNoise, config::WORLD_BIOME);

    std::lock_guard<std::mutex> guard(chunkManager.chunksMutex);
    for (uint32_t i = 0; i < numChunks; i++)
    {
        const uint64_t key = chunkPositions[i];
        assert(!chunkManager.chunks.contains(key) && "Duplicate chunk key");
        chunkManager.chunks[key] = newChunks[i];
    }
}

void ChunkManager::loadChunks()
{
    if (numChunksToLoad == 0) return;

    const uint32_t chunksPerThread = std::max(1u, numChunksToLoad / threadPool.getThreadCount());

    Chunk* newChunks[numChunksToLoad] = {nullptr};
    std::latch loadLatch(numChunksToLoad);

    for (uint32_t i = 0; i < numChunksToLoad; i += chunksPerThread)
    {
        const uint32_t batchSize = (i + chunksPerThread) > numChunksToLoad ? numChunksToLoad - i : chunksPerThread;
        threadPool.queueJob([baseIndex = i, batchSize, this, &newChunks, &loadLatch]()
        {
            const FastNoiseLite localNoise = createBiomeNoise(config::WORLD_BIOME, config::WORLD_SEED);

            for (uint32_t j = 0; j < batchSize; j++)
            {
                const uint32_t chunkIndex = baseIndex + j;
                const glm::uvec2 chunkPos = unpackChunkPos(chunksToLoad[chunkIndex]);
                newChunks[chunkIndex] = new Chunk(chunkPos, localNoise, config::WORLD_BIOME);
                loadLatch.count_down();
            }
        });
    }

    loadLatch.wait();

    for (uint32_t i = 0; i < numChunksToLoad; i++)
    {
        const uint64_t key = chunksToLoad[i];
        if (!chunks.contains(key))
            chunks[key] = newChunks[i];
        else
            LOG_WARN("Attempted to load a chunk that already exists at key: {}", key);
    }

    numChunksToLoad = 0;
}

Chunk* ChunkManager::getChunk(const glm::uvec2 pos)
{
    const auto key = packChunkPos(pos);
    std::lock_guard<std::mutex> guard(chunksMutex);
    const auto it = chunks.find(key);
    if (it == chunks.end())
        return nullptr;
    return it->second;
}

void ChunkManager::dropChunkMeshes()
{
    std::lock_guard<std::mutex> guard(chunksMutex);
    for (auto& [_, chunk] : chunks)
        chunk->isDirty = true;
}

uint32_t ChunkManager::getChunkCount() const
{
    return chunks.size();
}

// CHUNK ---------------------------------------

static uint32_t getBlockIndex(const glm::uvec3& pos) { return pos.x + pos.y * Chunk::CHUNK_SIZE + pos.z * Chunk::CHUNK_SIZE * Chunk::MAX_HEIGHT; }

uint32_t noiseToHeight(const float value)
{
    const float normalized = (value + 1.0f) * 0.5f;
    return Chunk::MIN_GEN_HEIGHT + normalized * (Chunk::MAX_GEN_HEIGHT - Chunk::MIN_GEN_HEIGHT);
}

Chunk::Chunk()
    : chunkPosition({0}), blocks{}
{
}

Chunk::Chunk(const glm::uvec2& chunkPosition, const FastNoiseLite& noise, const BIOME biome)
    : chunkPosition(chunkPosition), blocks{}
{
    const BLOCK_TYPE defaultBlock = defaultBiomeBlock(biome);
    for (uint32_t x = 0; x < CHUNK_SIZE; x++)
    {
        for (uint32_t z = 0; z < CHUNK_SIZE; z++)
        {
            const glm::uvec3 worldPos = chunkPosToWorldBlockPos(chunkPosition) + glm::uvec3{x, 0, z};
            const uint32_t localHeight = noiseToHeight(noise.GetNoise((float) worldPos.x, (float) worldPos.z));

            for (uint32_t y = 0; y < MAX_HEIGHT; y++)
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

void Chunk::bake(ChunkManager& chunkManager)
{
    std::vector<blockdata> buffer;
    uint32_t faceCount = 0;
    for (uint32_t x = 0; x < CHUNK_SIZE; x++)
    {
        for (uint32_t y = 0; y < MAX_HEIGHT; y++)
        {
            for (uint32_t z = 0; z < CHUNK_SIZE; z++)
            {
                const auto& block = getBlockUnsafe({x,y,z});

                assert(block != BLOCK_TYPE::INVALID);
                if (block == BLOCK_TYPE::AIR)
                    continue;

                for (uint32_t i = 0; i < 6; i++)
                {
                    if (i == BOTTOM && y == 0)
                        continue;
                    if (i == TOP && y == MAX_HEIGHT - 1)
                        continue;

                    glm::uvec3 neighbourBlockPos;
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
                    if (neighbourBlock == BLOCK_TYPE::INVALID && neighbourBlockPos.y < MAX_HEIGHT)
                    {
                        // check if a neighboring chunk has a covering block
                        glm::uvec3 blockPosInOtherChunk = neighbourBlockPos;
                        glm::uvec2 neighbourChunkPos = chunkPosition;
                        if (neighbourBlockPos.x == CHUNK_SIZE) { neighbourChunkPos.x += 1; blockPosInOtherChunk.x = 0; }
                        else if (neighbourBlockPos.x > CHUNK_SIZE) { neighbourChunkPos.x -= 1; blockPosInOtherChunk.x = CHUNK_SIZE - 1; }

                        if (neighbourBlockPos.z == CHUNK_SIZE) { neighbourChunkPos.y += 1; blockPosInOtherChunk.z = 0; }
                        else if (neighbourBlockPos.z > CHUNK_SIZE) { neighbourChunkPos.y -= 1; blockPosInOtherChunk.z = CHUNK_SIZE - 1; }

                        assert(neighbourChunkPos != chunkPosition);

                        const Chunk* neighborChunk = chunkManager.getChunk(neighbourChunkPos);
                        assert(!neighborChunk || neighborChunk->getBlockSafe(blockPosInOtherChunk) != BLOCK_TYPE::INVALID);
                        if (neighborChunk && neighborChunk->getBlockUnsafe(blockPosInOtherChunk) != BLOCK_TYPE::AIR)
                            continue;
                    }

                    const glm::uvec2 atlasOffset = getAtlasOffset(block, i);
                    blockdata packedData = (i << 28) | (x << 24) | (y << 16) | (z << 12) | (atlasOffset.x << 8) | (atlasOffset.y << 4);
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
    isDirty = false;
}

BLOCK_TYPE Chunk::getBlockUnsafe(const glm::uvec3& pos) const
{
    return blocks[getBlockIndex(pos)];
}

BLOCK_TYPE Chunk::getBlockSafe(const glm::uvec3& pos) const
{
    if (inBounds(pos))
        return getBlockUnsafe(pos);

    return BLOCK_TYPE::INVALID;
}

void Chunk::setBlockUnsafe(const glm::uvec3& pos, const BLOCK_TYPE block)
{
    blocks[getBlockIndex(pos)] = block;
    isDirty = true;
}

void Chunk::setBlockSafe(const glm::uvec3& pos, BLOCK_TYPE block)
{
    if (inBounds(pos))
    {
        setBlockUnsafe(pos, block);
        isDirty = true;
    }
    else
        LOG_WARN("BLock not found in chunk: ({}, {}, {})", pos.x, pos.y, pos.z);
}
