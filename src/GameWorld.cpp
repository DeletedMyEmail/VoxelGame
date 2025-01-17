#include "GameWorld.h"

#include "Log.h"
#include "Noise.h"

GameWorld::GameWorld(const glm::vec3 playerPos, const unsigned int chunksPerSide)
    : m_Player(playerPos)
{
    initChunks(chunksPerSide);
}

void GameWorld::playerCollisions()
{
    PhysicsBody& playerBody = m_Player.getPhysics();
    // just in 1 chunk
    const auto chunkPos = glm::uvec2(static_cast<unsigned int>(playerBody.getPosition().x) / Chunk::CHUNK_SIZE, static_cast<unsigned int>(playerBody.getPosition().z) / Chunk::CHUNK_SIZE);
    const Chunk* relevantChunk = nullptr;


    for (auto& chunk : m_Chunks)
    {
        if (chunk.getPosition() == chunkPos)
        {
            relevantChunk = &chunk;
            break;
        }
    }

    if (relevantChunk == nullptr)
        assert(false);

    const unsigned char relativeX = static_cast<unsigned int>(playerBody.getPosition().x) - chunkPos.x * Chunk::CHUNK_SIZE;
    const unsigned char relativeY = static_cast<unsigned char>(playerBody.getPosition().y);
    const unsigned char relativeZ = static_cast<unsigned int>(playerBody.getPosition().z) - chunkPos.y * Chunk::CHUNK_SIZE;

    for (unsigned int x = glm::clamp(relativeX-1, 0, Chunk::CHUNK_SIZE-2); x < relativeX+1; x++)
    {
        for (unsigned int y = glm::clamp(relativeY-1, 0, Chunk::MAX_HEIGHT-2); y < relativeY+1; y++)
        {
            for (unsigned int z = glm::clamp(relativeZ-1, 0, Chunk::CHUNK_SIZE-2); z < relativeZ+1; z++)
            {
                auto& block = relevantChunk->getBlock({x,y,z});
                if (block.type == AIR)
                    continue;

                PhysicsBody blockBounding({x,y,z}, {1,1,1}, true);
                if (playerBody.solveCollision(blockBounding))
                {
                    LOG_INFO("WOW A COLLISION");
                }
            }
        }
    }

    // TODO: edge case: in multiple chunks at once
}

void GameWorld::update()
{
    playerCollisions();
    m_Player.getPhysics().update();
}

void GameWorld::initChunks(const unsigned int chunksPerSide)
{
    const unsigned int size = Chunk::CHUNK_SIZE * chunksPerSide;
    unsigned char** heightMap = genPerlinMap(size, size, Chunk::MAX_HEIGHT / 2, Chunk::MAX_HEIGHT, 42);

    m_Chunks.reserve(chunksPerSide * chunksPerSide);
    for (unsigned int x = 0; x < chunksPerSide; x++)
    {
        for (unsigned int z = 0; z < chunksPerSide; z++)
        {
            m_Chunks.emplace_back(glm::uvec2{x, z}, heightMap);
        }
    }

    freeMap(heightMap, size);
}
