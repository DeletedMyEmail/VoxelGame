#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include <cstmlib/Profiling.h>
#include "imgui.h"
#include "Camera.h"
#include "Chunk.h"
#include "Window.h"
#include "OpenGLHelper.h"
#include "Raycast.h"
#include "ThreadPool.h"
#include <deque>
#include <numeric>
#include <algorithm>

#include "Physics.h"
#include "Rendering.h"
#include "WorldGeneration.h"
#include "stb/stb_image.h"

glm::vec3 moveInput(const Window& window, const glm::vec3& lookDir);
void placeBlock(ChunkManager& chunkManager, Camera& cam, BLOCK_TYPE block);
void handleCollisions(ChunkManager& chunkManager, Camera& cam, const glm::vec3& vel);

int main(int argc, char* argv[])
{
    LOG_INIT();
    PROFILER_INIT(100);

    bool debugMode = true, cursorLocked = true;
    MenuSettings menuSettings{
        BLOCK_TYPE::INVALID,
        50.0f,
        0.8f,
        true
    };

    Window window;
    window.setCursorDisabled(cursorLocked);
    renderConfig(window.getGLFWWindow());

    Camera cam(glm::vec3{0, Chunk::CHUNK_SIZE * WORLD_HEIGHT + 1, 0}, 90.0f, window.getWidth(), window.getHeight(), 0.1f, config::RENDER_DISTANCE * Chunk::CHUNK_SIZE * 4);

    ChunkManager chunkManager;
    glm::dvec2 prevCursorPos = window.getMousePosition();
    window.onKey([&chunkManager, &debugMode, &cursorLocked] (Window* win, const int key, const int scancode, const int action, const int mods)
        {
            if (action != GLFW_PRESS)
                return;
            if (key == GLFW_KEY_ESCAPE)
                win->stop();
            else if (key == GLFW_KEY_TAB)
                debugMode = !debugMode;
            else if (key == GLFW_KEY_V)
            {
                cursorLocked = !cursorLocked;
                win->setCursorDisabled(cursorLocked);
            }
            else if (key == GLFW_KEY_X)
                chunkManager.dropChunkMeshes();

        });
    
    window.onCursorMove([&cam, &prevCursorPos, &cursorLocked](Window* win, const glm::dvec2 pos)
        {
            ImGui::GetIO().MousePos = ImVec2(float(pos.x), float(pos.y));
            const glm::dvec2 offset = pos - prevCursorPos;
            if (cursorLocked)
                cam.rotate({offset.x, offset.y});
            prevCursorPos = pos;
        });

    window.onMouseButton([&cam, &chunkManager, &cursorLocked, &menuSettings](Window* win, int button, int action, int mods)
        {
            ImGui::GetIO().MouseDown[button] = (action == GLFW_PRESS);
            if (action != GLFW_PRESS || !cursorLocked)
                return;

            if (button == GLFW_MOUSE_BUTTON_LEFT)
                placeBlock(chunkManager, cam, menuSettings.selectedBlock);
        });

    Metrics metrics;
    while (window.isRunning())
    {
        metrics.update();

        float skyExposure = 0.5f + 0.5f * menuSettings.exposure;
        clearFrame(skyExposure, debugMode);

        const glm::vec3 dir = moveInput(window, cam.lookDir);
        const glm::vec3 vel = dir * metrics.deltaTime * menuSettings.camSpeed;

        auto chunkPos = worldPosToChunkPos(cam.position);
        TIME(metrics, "Collision Detection", ({
            if (!menuSettings.collisionsOn)
                cam.translate(vel);
            else if (vel != glm::vec3(0))
                handleCollisions(chunkManager, cam, vel);
        }));
        cam.updateView();
        TIME(metrics, "Chunk Unloading", chunkManager.unloadChunks(chunkPos));
        TIME(metrics, "Chunk Loading", chunkManager.loadChunks(chunkPos));
        TIME(metrics, "Chunk Baking", chunkManager.bakeChunks(chunkPos));
        TIME(metrics, "Chunk Drawing", chunkManager.drawChunks(cam.viewProjection, menuSettings.exposure));
        TIME(metrics, "Block Highlighting",
             RaycastResult res = raycast(cam.position, cam.lookDir, config::REACH_DISTANCE, chunkManager);
             if (res.hit)
             drawHighlightBlock(worldPosToChunkBlockPos(res.pos), chunkPosToWorldBlockPos(res.chunk->chunkPosition), cam.viewProjection, menuSettings.exposure);
        );

        if (debugMode)
        {
            drawAxes(cam);
            drawDebugMenu(metrics, menuSettings, cam.position);
        }

        glfwSwapBuffers(window.getGLFWWindow());
        glfwPollEvents();
    }

    PROFILER_END();
    return 0;
}

glm::vec3 moveInput(const Window& window, const glm::vec3& lookDir)
{
    glm::vec3 input(0.0f);
    const float forwardInput =  1.0f * window.isKeyDown(GLFW_KEY_W) - 1.0f * window.isKeyDown(GLFW_KEY_S);
    const float rightInput   =  1.0f * window.isKeyDown(GLFW_KEY_D) - 1.0f * window.isKeyDown(GLFW_KEY_A);
    const float upInput      =  1.0f * window.isKeyDown(GLFW_KEY_SPACE) - 1.0f * window.isKeyDown(GLFW_KEY_LEFT_SHIFT);

    if (forwardInput == 0 && rightInput == 0 && upInput == 0)
        return glm::vec3(0.0f);

    glm::vec3 forward = glm::normalize(glm::vec3(lookDir.x, 0.0f, lookDir.z));
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

    input += forward * forwardInput;
    input += right * rightInput;
    input.y += upInput;

    assert(std::isnan(input.x) == false && std::isnan(input.y) == false && std::isnan(input.z) == false);

    return glm::normalize(input);
}

void placeBlock(ChunkManager& chunkManager, Camera& cam, BLOCK_TYPE block)
{
    assert(block != BLOCK_TYPE::INVALID);

    RaycastResult res = raycast(cam.position, cam.lookDir, config::REACH_DISTANCE, chunkManager);
    if (!res.hit)
        return;

    const auto positionInChunk = worldPosToChunkBlockPos(res.pos);

    if (block == BLOCK_TYPE::AIR)
        res.chunk->setBlockUnsafe(positionInChunk, BLOCK_TYPE::AIR);
    else
    {
        glm::ivec3 offset;
        switch (res.face)
        {
        case BACK: offset = {0,0,-1}; break;
        case FRONT: offset = {0,0,1}; break;
        case LEFT: offset = {-1,0,0}; break;
        case RIGHT: offset = {1,0,0}; break;
        case TOP: offset = {0, 1, 0}; break;
        case BOTTOM: offset = {0, -1, 0}; break;
        default: assert(false);
        }
        glm::ivec3 neighbourBlockPos = positionInChunk + offset;

        if (isChunkCoord(neighbourBlockPos))
            res.chunk->setBlockUnsafe(neighbourBlockPos, block);
        else
        {
            glm::uvec3 blockPosInOtherChunk = neighbourBlockPos;
            if (neighbourBlockPos.x == Chunk::CHUNK_SIZE) blockPosInOtherChunk.x = 0;
            else if (neighbourBlockPos.x == -1) blockPosInOtherChunk.x = Chunk::CHUNK_SIZE - 1;
            if (neighbourBlockPos.z == Chunk::CHUNK_SIZE) blockPosInOtherChunk.z = 0;
            else if (neighbourBlockPos.z == -1) blockPosInOtherChunk.z = Chunk::CHUNK_SIZE - 1;


            Chunk* neighbourChunk = chunkManager.getChunk(res.chunk->chunkPosition + offset);

            assert(neighbourChunk != nullptr);
            assert(neighbourChunk->getBlockSafe(blockPosInOtherChunk) != BLOCK_TYPE::INVALID);
            neighbourChunk->setBlockUnsafe(blockPosInOtherChunk, block);
        }
    }

    if (positionInChunk.x == 0)
    {
        Chunk* chunk = chunkManager.getChunk({res.chunk->chunkPosition.x - 1, res.chunk->chunkPosition.y, res.chunk->chunkPosition.z});
        if (chunk) chunk->isMeshBaked = false;
    }
    else if (positionInChunk.x == Chunk::CHUNK_SIZE - 1)
    {
        Chunk* chunk = chunkManager.getChunk({res.chunk->chunkPosition.x + 1, res.chunk->chunkPosition.y, res.chunk->chunkPosition.z});
        if (chunk) chunk->isMeshBaked = false;
    }
    if (positionInChunk.y == 0)
    {
        Chunk* chunk = chunkManager.getChunk({res.chunk->chunkPosition.x, res.chunk->chunkPosition.y - 1, res.chunk->chunkPosition.z});
        if (chunk) chunk->isMeshBaked = false;
    }
    else if (positionInChunk.y == Chunk::CHUNK_SIZE - 1)
    {
        Chunk* chunk = chunkManager.getChunk({res.chunk->chunkPosition.x, res.chunk->chunkPosition.y + 1, res.chunk->chunkPosition.z});
        if (chunk) chunk->isMeshBaked = false;
    }
    if (positionInChunk.z == 0)
    {
        Chunk* chunk = chunkManager.getChunk({res.chunk->chunkPosition.x, res.chunk->chunkPosition.y, res.chunk->chunkPosition.z - 1});
        if (chunk) chunk->isMeshBaked = false;
    }
    else if (positionInChunk.z == Chunk::CHUNK_SIZE - 1)
    {
        Chunk* chunk = chunkManager.getChunk({res.chunk->chunkPosition.x, res.chunk->chunkPosition.y, res.chunk->chunkPosition.z + 1});
        if (chunk) chunk->isMeshBaked = false;
    }
}

void handleCollisions(ChunkManager& chunkManager, Camera& cam, const glm::vec3& vel)
{
    PhysicsObject playerPhysics{};
    playerPhysics.box.pos = cam.position - glm::vec3{0.5f, 1.0f, 0.5f};
    playerPhysics.box.size = glm::vec3(1.0f, 1.0f, 1.0f);
    playerPhysics.velocity = vel;
    auto [broadPos, broadSize] = getBroadphaseBox(playerPhysics);

    CollisionData collisionData{glm::vec3(0), std::numeric_limits<float>::max()};
    BoundingBox collidingBlock{glm::vec3(0), glm::vec3(1.0f)};

    for (int32_t x = glm::floor(broadPos.x); x < glm::ceil(broadPos.x + broadSize.x); ++x)
    {
        for (int32_t y = glm::floor(broadPos.y); y < glm::ceil(broadPos.y + broadSize.y); ++y)
        {
            for (int32_t z = glm::floor(broadPos.z); z < glm::ceil(broadPos.z + broadSize.z); ++z)
            {
                glm::ivec3 worldPos{x, y, z};
                Chunk* chunk = chunkManager.getChunk(worldPosToChunkPos(worldPos));
                if (!chunk)
                    continue;
                glm::ivec3 blockPos = worldPosToChunkBlockPos(worldPos);
                BLOCK_TYPE block = chunk->getBlockSafe(blockPos);
                if (block == BLOCK_TYPE::INVALID || !isSolid(block))
                    continue;

                BoundingBox blockBox{worldPos, glm::vec3(1.0f)};
                CollisionData c = getCollision(playerPhysics, blockBox);

                if (c.entryTime < collisionData.entryTime)
                {
                    collisionData = c;
                    collidingBlock.pos = worldPos;
                }
            }
        }
    }

    if (collisionData.entryTime < std::numeric_limits<float>::max())
    {
        resolveCollision(playerPhysics, collidingBlock, collisionData);
        glm::vec3 collisionOffset = cam.position - (playerPhysics.box.pos + glm::vec3(0.5f, 1.0f, 0.5f));
        cam.translate(collisionOffset);
    }
    else
    {
        cam.translate(vel);
    }
}