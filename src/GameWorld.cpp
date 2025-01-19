#include "GameWorld.h"

#include <App.h>
#include <App.h>
#include <Window.h>

#include "Log.h"
#include "Noise.h"

GameWorld::GameWorld(const glm::vec3 playerPos, const unsigned int chunksPerSide)
    : m_Player(playerPos)
{
    createChunks(chunksPerSide);
}

void GameWorld::playerCollisions()
{
    PhysicsBody& playerBody = m_Player.getPhysics();

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

    const unsigned int relativeX = playerBody.getPosition().x - chunkPos.x * Chunk::CHUNK_SIZE;
    const unsigned int relativeY = playerBody.getPosition().y;
    const unsigned int relativeZ = playerBody.getPosition().z - chunkPos.y * Chunk::CHUNK_SIZE;

    const unsigned int xStart = glm::clamp(relativeX, 1u, Chunk::CHUNK_SIZE-4)-1;
    const unsigned int yStart = glm::clamp(relativeY, 1u, Chunk::MAX_HEIGHT-4)-1;
    const unsigned int zStart = glm::clamp(relativeZ, 1u, Chunk::CHUNK_SIZE-4)-1;

    for (unsigned int x = xStart; x < xStart+4; x++)
    {
        for (unsigned int y = yStart; y < yStart+4; y++)
        {
            for (unsigned int z = zStart; z < zStart+4; z++)
            {
                const auto& block = relevantChunk->getBlock({x,y,z});
                if (block.type == AIR)
                    continue;

                const PhysicsBody blockBounding({x,y,z}, {1,1,1}, true);
                if (playerBody.solveCollision(blockBounding))
                {

                }
            }
        }
    }

    // TODO: player in multiple chunks at once
}

glm::vec3 getPlayerMoveInputs(const Window& win)
{
    glm::vec3 movement(0.0f);
    if (win.isKeyDown(GLFW_KEY_W)) movement.z += 1.0f;
    if (win.isKeyDown(GLFW_KEY_S)) movement.z -= 1.0f;
    if (win.isKeyDown(GLFW_KEY_A)) movement.x += 1.0f;
    if (win.isKeyDown(GLFW_KEY_D)) movement.x -= 1.0f;
    if (win.isKeyDown(GLFW_KEY_SPACE)) movement.y += 1.0f;
    if (win.isKeyDown(GLFW_KEY_LEFT_SHIFT)) movement.y -= 1.0f;

    return movement;
}

void GameWorld::update(const float deltaTime, const Window& win, const glm::vec3 lookDir)
{
    const glm::vec3 input = getPlayerMoveInputs(win);
    glm::vec3 vel;
    vel = lookDir * input.z;
    vel -= normalize(cross(lookDir, glm::vec3(0.0f, 1.0f, 0.0f))) * input.x;
    vel.y = input.y;

    PhysicsBody& playerBody = m_Player.getPhysics();
    playerBody.addVelocity(vel * deltaTime * m_Player.getSpeed());
    playerCollisions();
    playerBody.update();
}

void GameWorld::createChunks(const unsigned int chunksPerSide)
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
