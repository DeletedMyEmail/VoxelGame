#include "Chunk.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <ranges>
#include "glm/gtx/norm.hpp"
#include "Block.h"
#include "OpenGLHelper.h"
#include "GameWorld.h"
#include "cstmlib/Profiling.h"
#include "glm/common.hpp"

ChunkManager::ChunkManager(const GameConfig& config)
    : m_ThreadPool(config.threadCount), m_Config(config), m_WorldGenData(config.worldSeed)
{
    m_Chunks.reserve((2 * config.loadDistance) * (2 * config.loadDistance) * (WorldGenerationData::WORLD_HEIGHT));
}

void ChunkManager::unloadChunks(const glm::ivec3& currChunkPos)
{
    uint32_t unloads = 0;
    for (auto it = m_Chunks.begin(); it != m_Chunks.end();)
    {
        Chunk& chunk = it->second;

        const int32_t xDist = glm::abs(chunk.chunkPosition.x - currChunkPos.x);
        const int32_t yDist = glm::abs(chunk.chunkPosition.y - currChunkPos.y);
        const int32_t zDist = glm::abs(chunk.chunkPosition.z - currChunkPos.z);

        if (unloads < m_Config.maxUnloadsPerFrame &&
            !chunk.inRender &&
            (xDist > m_Config.loadDistance || yDist > m_Config.loadDistance || zDist > m_Config.loadDistance))
        {
            it = m_Chunks.erase(it);
            unloads++;
        }
        else
        {
            ++it;

            chunk.inRender =
                xDist < m_Config.renderDistance &&
                yDist < m_Config.renderDistance &&
                zDist < m_Config.renderDistance;
        }
    }
}

struct ChunkLoadRequest
{
    glm::ivec3 position;
    float priority;

    bool operator<(const ChunkLoadRequest& other) const { return priority > other.priority; }
};

std::priority_queue<ChunkLoadRequest> getChunksSorted(const glm::ivec3& currChunkPos, const int32_t maxDist)
{
    uint32_t chunksAdded = 0;
    std::priority_queue<ChunkLoadRequest> queue;
    for (int32_t x = currChunkPos.x - maxDist; x <= currChunkPos.x + maxDist; x++)
    {
        for (int32_t y = glm::max(0, currChunkPos.y - maxDist); y <= WorldGenerationData::WORLD_HEIGHT - 1 && y < currChunkPos.y + maxDist; y++)
        {
            for (int32_t z = currChunkPos.z - maxDist; z <= currChunkPos.z + maxDist; z++)
            {
                glm::ivec3 chunkPos = {x, y, z};
                const float distance = glm::length2(glm::vec3(chunkPos - currChunkPos));
                queue.emplace(chunkPos, distance);
                chunksAdded++;
            }
        }
    }

    return queue;
}

void ChunkManager::drawChunks(Renderer& renderer, const glm::mat4& viewProjection, const float exposure)
{
    renderer.prepareChunkRendering(viewProjection, exposure);

    for (const auto& [_,chunk] : m_Chunks)
        if (chunk.inRender && chunk.isMeshBaked)
            renderer.drawChunk(chunk.vaoOpaque, chunkPosToWorldBlockPos(chunk.chunkPosition));

    glDisable(GL_CULL_FACE);
    for (const auto& [_,chunk] : m_Chunks)
        if (chunk.inRender && chunk.isMeshBaked)
            renderer.drawChunk(chunk.vaoTranslucent, chunkPosToWorldBlockPos(chunk.chunkPosition));
    glEnable(GL_CULL_FACE);
}

void ChunkManager::bakeChunks(const glm::ivec3& currChunkPos)
{
    uint32_t chunksBaked = 0;
    glm::ivec3 chunkPositions[m_Config.maxLoadsPerFrame];
    auto chunkQueue = getChunksSorted(currChunkPos, m_Config.renderDistance);

    while (!chunkQueue.empty())
    {
        auto [position, priority] = chunkQueue.top();
        chunkQueue.pop();

        auto it = m_Chunks.find(position);
        if (it == m_Chunks.end())
            continue;

        Chunk& chunk = it->second;
        if (chunk.isMeshBaked || chunksBaked >= m_Config.maxBakesPerFrame)
            continue;

        chunkPositions[chunksBaked] = position;
        chunksBaked++;
        m_ThreadPool.queueJob([this, &chunk, position]()
        {
            std::array<Chunk*, 6> neighbourChunks{
                // BACK, FRONT, LEFT, RIGHT, BOTTOM, TOP
                getChunk(position + glm::ivec3{0, 0, -1}),
                getChunk(position + glm::ivec3{0, 0, 1}),
                getChunk(position + glm::ivec3{-1, 0, 0}),
                getChunk(position + glm::ivec3{1, 0, 0}),
                getChunk(position + glm::ivec3{0, -1, 0}),
                getChunk(position + glm::ivec3{0, 1, 0})
            };

            chunk.generateMeshData(neighbourChunks);
        });
    }

    while (m_ThreadPool.busy())
        std::this_thread::sleep_for(std::chrono::microseconds(1));

    // TODO: wieso durch alle chunks?
    for (uint32_t i = 0; i < chunksBaked; i++)
    {
        Chunk& chunk = (*m_Chunks.find(chunkPositions[i])).second;
        chunk.bakeMesh();
    }

    //LOG_INFO("{} chunks baked", chunksBaked);
}

void ChunkManager::loadChunks(const glm::ivec3& currChunkPos, SQLite::Database& db)
{
    auto chunkQueue = getChunksSorted(currChunkPos, m_Config.loadDistance);
    glm::ivec3 chunkPositionsOfLoaded[m_Config.maxLoadsPerFrame];
    uint32_t chunksLoaded = 0;
    while (!chunkQueue.empty() && chunksLoaded < m_Config.maxLoadsPerFrame)
    {
        auto [position, priority] = chunkQueue.top();
        chunkQueue.pop();

        if (m_Chunks.find(position) != m_Chunks.end())
            continue;

        chunkPositionsOfLoaded[chunksLoaded++] = position;
        m_Chunks.emplace(std::piecewise_construct, std::forward_as_tuple(position), std::forward_as_tuple());
    }

    for (uint32_t i = 0; i < chunksLoaded; i++)
    {
        const glm::ivec3& position = chunkPositionsOfLoaded[i];
        Chunk* chunk = &m_Chunks.at(position);
        m_ThreadPool.queueJob([this, position, chunk]()
        {
            *chunk = Chunk(position, m_WorldGenData);
        });
    }

    while (m_ThreadPool.busy())
        std::this_thread::sleep_for(std::chrono::microseconds(1));

    for (uint32_t i = 0; i < chunksLoaded; i++)
    {
        auto changes = getBlockChangesForChunk(db, chunkPositionsOfLoaded[i]);
        Chunk* chunk = getChunk(chunkPositionsOfLoaded[i]);
        for (const auto& [positionInChunk, blockType] : changes)
            chunk->setBlockUnsafe(positionInChunk, blockType);
    }

    //LOG_INFO("{} chunks loaded", chunksLoaded);
}

void ChunkManager::dropChunkMeshes()
{
    for(auto& chunk : m_Chunks | std::views::values)
        chunk.isMeshBaked = false;
}

Chunk* ChunkManager::getChunk(const glm::ivec3& pos)
{
    const auto it = m_Chunks.find(pos);
    if (it != m_Chunks.end())
        return &it->second;
    return nullptr;
}

// CHUNK ---------------------------------------

static uint32_t getBlockIndex(const glm::ivec3& pos) { return pos.x + pos.y * Chunk::CHUNK_SIZE + pos.z * Chunk::CHUNK_SIZE * Chunk::CHUNK_SIZE; }

Chunk::Chunk()
    : blocks{}, chunkPosition(0)
{
}

const int32_t TREE_HEIGHT = 6;

void Chunk::spawnTree(const glm::ivec3& pos)
{
    for (int32_t y = 0; y < TREE_HEIGHT; y++)
        setBlockUnsafe({pos.x, pos.y + y, pos.z}, BLOCK_TYPE::WOOD);

    for (int32_t x = -1; x <= 1; x++)
        for (int32_t z = -1; z <= 1; z++)
            setBlockUnsafe({pos.x + x, pos.y + TREE_HEIGHT - 1, pos.z + z}, BLOCK_TYPE::LEAVES);
}

Chunk::Chunk(const glm::ivec3& chunkPosition, const WorldGenerationData& worldGenData)
    : blocks{}, chunkPosition(chunkPosition)
{
    meshDataOpaque.reserve(BLOCKS_PER_CHUNK / 2);
    meshDataTranslucent.reserve(BLOCKS_PER_CHUNK / 2);

    const glm::ivec3 absChunkPos = chunkPosToWorldBlockPos(chunkPosition);
    const int32_t chunkHeight = chunkPosition.y * CHUNK_SIZE;
    bool forestChunk = worldGenData.isForest({absChunkPos.x, absChunkPos.z});
    const int32_t SURFACE_HEIGHT = 3;


    for (int32_t x = 0; x < CHUNK_SIZE; x++)
    {
        for (int32_t z = 0; z < CHUNK_SIZE; z++)
        {
            glm::ivec3 absPos = absChunkPos + glm::ivec3{x, 0, z};
            const int32_t terrainHeight = worldGenData.getHeightAt({absPos.x, absPos.z});
            BLOCK_TYPE surfaceBlock = BLOCK_TYPE::GRASS;

            if (terrainHeight < WorldGenerationData::SEA_LEVEL + 3)
                surfaceBlock = BLOCK_TYPE::SAND;
            else if (terrainHeight < WorldGenerationData::SEA_LEVEL + 5)
                surfaceBlock = BLOCK_TYPE::STONE;

            if (forestChunk &&
                x > 0 && z > 0 &&
                x < CHUNK_SIZE - 1 && z < CHUNK_SIZE - 1 &&
                WorldGenerationData::SEA_LEVEL < terrainHeight &&
                terrainHeight >= chunkHeight &&
                terrainHeight + TREE_HEIGHT < chunkHeight + CHUNK_SIZE)
            {
                const bool tree = worldGenData.hasTree({absPos.x, absPos.z});
                if (tree)
                {
                    int32_t trunkY = terrainHeight - chunkHeight;
                    spawnTree(glm::ivec3{x, trunkY, z});
                }
            }

            for (uint32_t y = 0; y < CHUNK_SIZE; y++)
            {
                const int32_t index = getBlockIndex({x,y,z});
                if (blocks[index] != BLOCK_TYPE::INVALID)
                    continue;


                const int32_t absY = y + chunkHeight;

                if (absY >= terrainHeight)
                {
                    if (absY <= WorldGenerationData::SEA_LEVEL && terrainHeight <= WorldGenerationData::SEA_LEVEL)
                        blocks[index] = BLOCK_TYPE::WATER;
                    else
                        blocks[index] = BLOCK_TYPE::AIR;
                }
                else
                    blocks[index] = absY >= terrainHeight - SURFACE_HEIGHT ? surfaceBlock : BLOCK_TYPE::STONE;
            }
        }
    }
}

void Chunk::generateMeshData(const std::array<Chunk*, 6>& neighbourChunks)
{
    meshDataOpaque.clear();
    meshDataTranslucent.clear();

    constexpr glm::ivec3 neighborOffsets[] = {
        {0, 0, -1}, // BACK
        {0, 0, 1},  // FRONT
        {-1, 0, 0}, // LEFT
        {1, 0, 0},  // RIGHT
        {0, -1, 0},  // BOTTOM
        {0, 1, 0}  // TOP
    };

    for (uint32_t z = 0; z < CHUNK_SIZE; z++)
    {
        for (uint32_t y = 0; y < CHUNK_SIZE; y++)
        {
            for (uint32_t x = 0; x < CHUNK_SIZE; x++)
            {
                glm::uvec3 blockPos = {x, y, z};
                const BLOCK_TYPE block = getBlockUnsafe(blockPos);

                assert(block != BLOCK_TYPE::INVALID);
                if (block == BLOCK_TYPE::AIR)
                    continue;

                for (uint32_t face = 0; face < 6; face++)
                {
                    if (face == BOTTOM && y == 0)
                        continue;

                    glm::ivec3 neighbourBlockPos = glm::ivec3(blockPos) + neighborOffsets[face];

                    BLOCK_TYPE neighbourBlock = getBlockSafe(neighbourBlockPos);
                    if (neighbourBlock != BLOCK_TYPE::INVALID && neighbourBlock != BLOCK_TYPE::AIR && !(!isTranslucent(block) && isTranslucent(neighbourBlock)))
                        continue;

                    if (neighbourBlock == BLOCK_TYPE::INVALID)
                    {
                        glm::uvec3 blockPosInOtherChunk;
                        blockPosInOtherChunk.x = (neighbourBlockPos.x % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
                        blockPosInOtherChunk.y = (neighbourBlockPos.y % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
                        blockPosInOtherChunk.z = (neighbourBlockPos.z % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;

                        Chunk* neighborChunk = neighbourChunks[face];

                        if (neighborChunk)
                        {
                            neighbourBlock = neighborChunk->getBlockUnsafe(blockPosInOtherChunk);
                            if (neighbourBlock != BLOCK_TYPE::AIR && !(block != BLOCK_TYPE::WATER && neighbourBlock == BLOCK_TYPE::WATER))
                                continue;
                        }
                    }

                    auto atlasOffset = getAtlasOffset(block, FACE(face));
                    if (isTranslucent(block))
                        meshDataTranslucent.push_back(packBlockData(blockPos, atlasOffset, FACE(face)));
                    else
                        meshDataOpaque.push_back(packBlockData(blockPos, atlasOffset, FACE(face)));
                }
            }
        }
    }
}

void bake(VertexArray& vao, const std::vector<blockdata>& meshData)
{
    VertexBufferLayout layout;
    layout.pushUInt(1, false, 1);

    const GLuint instanceVbo = createBuffer(meshData.data(), meshData.size() * sizeof(blockdata));

    vao.reset();
    vao.addBuffer(instanceVbo, layout);
    vao.vertexCount = meshData.size();
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
}

void Chunk::setBlockSafe(const glm::ivec3& pos, const BLOCK_TYPE block)
{
    if (isChunkCoord(pos))
        setBlockUnsafe(pos, block);
    else
        LOG_WARN("Set: invalid block pos ({}, {}, {})", pos.x, pos.y, pos.z);
}

glm::ivec3 chunkPosToWorldBlockPos(const glm::ivec3& chunkPos)
{
    return chunkPos * Chunk::CHUNK_SIZE;
}

glm::ivec3 worldPosToChunkBlockPos(const glm::ivec3& worldPos)
{
    return {
        (worldPos.x % Chunk::CHUNK_SIZE + Chunk::CHUNK_SIZE) % Chunk::CHUNK_SIZE,
        (worldPos.y % Chunk::CHUNK_SIZE + Chunk::CHUNK_SIZE) % Chunk::CHUNK_SIZE,
        (worldPos.z % Chunk::CHUNK_SIZE + Chunk::CHUNK_SIZE) % Chunk::CHUNK_SIZE
    };
}

glm::ivec3 worldPosToChunkPos(const glm::ivec3& worldPos)
{
    return {
        int(std::floor(float(worldPos.x) / Chunk::CHUNK_SIZE)),
        int(std::floor(float(worldPos.y) / Chunk::CHUNK_SIZE)),
        int(std::floor(float(worldPos.z) / Chunk::CHUNK_SIZE))
    };
}

bool isChunkCoord(const glm::ivec3& pos)
{
    return pos.x < Chunk::CHUNK_SIZE && pos.x >= 0 &&
           pos.y < Chunk::CHUNK_SIZE && pos.y >= 0 &&
           pos.z < Chunk::CHUNK_SIZE && pos.z >= 0;
}
