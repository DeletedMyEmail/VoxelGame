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
#include <numeric>
#include "Physics.h"
#include "Rendering.h"
#include "GameWorld.h"
#include "SQLiteCpp/Database.h"
#include "stb/stb_image.h"
#include <libconfig.h++>

glm::vec3 moveInput(const Window& window, const glm::vec3& lookDir);
void placeBlock(ChunkManager& chunkManager, Camera& cam, BLOCK_TYPE block, SQLite::Database& db, float reachDistance);

int main(int argc, char* argv[])
{
#ifdef RELEASE_BUILD
    LOG_INIT_SPECIFIC(PROJECT_NAME, spdlog::level::warn, "log.txt");
#else
    LOG_INIT();
#endif
    PROFILER_INIT(100);

    ProgramConfig config;
    if (argc >= 2 && !loadConfig(argv[1], config))
    {
        LOG_ERROR("Failed to load config file: {}", argv[1]);
        return -1;
    }

    SQLite::Database db = initDB(config.saveGamePath);

    bool debugMode = true, cursorLocked = true;
    MenuSettings menuSettings{
        BLOCK_TYPE::INVALID,
        50.0f,
        0.8f,
        true
    };

    Window window;
    window.setCursorDisabled(cursorLocked);
    Renderer renderer(window.getGLFWWindow());

    Camera cam(glm::vec3{0, WorldGenerationData::MAX_HEIGHT + 1, 0}, 90.0f, window.getWidth(), window.getHeight(), 0.1f, config.renderDistance * Chunk::CHUNK_SIZE * 4);

    ChunkManager chunkManager(config);
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

    window.onMouseButton([&cam, &chunkManager, &cursorLocked, &menuSettings, &db, &config](Window* win, int button, int action, int mods)
        {
            ImGui::GetIO().MouseDown[button] = (action == GLFW_PRESS);
            if (action != GLFW_PRESS || !cursorLocked)
                return;

            if (button == GLFW_MOUSE_BUTTON_LEFT)
                placeBlock(chunkManager, cam, menuSettings.selectedBlock, db, config.reachDistance);
        });

    std::vector<Entity> entities;
    entities.emplace_back(glm::vec3{10.0f, WorldGenerationData::MAX_HEIGHT - 4, 10.0f}, glm::vec3{1.0f, 1.0f, 1.0f});
    entities.emplace_back(glm::vec3{10.0f, WorldGenerationData::MAX_HEIGHT - 4, 11.0f}, glm::vec3{1.0f, 2.0f, 1.0f});
    entities.emplace_back(glm::vec3{11.0f, WorldGenerationData::MAX_HEIGHT - 4, 10.0f}, glm::vec3{1.0f, 3.0f, 1.0f});
    entities.emplace_back(glm::vec3{11.0f, WorldGenerationData::MAX_HEIGHT - 4, 11.0f}, glm::vec3{1.0f, 4.0f, 1.0f});

    Metrics metrics;
    while (window.isRunning())
    {
        metrics.update();

        float skyExposure = 0.5f + 0.5f * menuSettings.exposure;
        renderer.clearFrame(skyExposure, debugMode);

        auto chunkPos = worldPosToChunkPos(cam.position);
        TIME(metrics, "Update Player", ({
            const glm::vec3 dir = moveInput(window, cam.lookDir);
            const glm::vec3 vel = dir * metrics.deltaTime * menuSettings.camSpeed;

            if (!menuSettings.collisionsOn)
                cam.move(vel);
            else if (vel != glm::vec3(0))
            {

                PhysicsObject playerPhysics{};
                playerPhysics.box.pos = cam.position - glm::vec3{0.5f, 1.0f, 0.5f};
                playerPhysics.box.size = glm::vec3(1.0f, 1.0f, 1.0f);
                playerPhysics.velocity = vel;
                handleCollision(chunkManager, playerPhysics);
                cam.position = playerPhysics.box.pos + glm::vec3(0.5f, 1.0f, 0.5f);

            }

            cam.updateView();
        }));

        TIME(metrics, "Chunk Unloading", chunkManager.unloadChunks(chunkPos));
        TIME(metrics, "Chunk Loading", chunkManager.loadChunks(chunkPos, db));
        TIME(metrics, "Chunk Baking", chunkManager.bakeChunks(chunkPos));
        TIME(metrics, "Chunk Drawing", chunkManager.drawChunks(renderer, cam.viewProjection, menuSettings.exposure));
        TIME(metrics, "Block Highlighting",
             RaycastResult res = raycast(cam.position - cam.lookDir, cam.lookDir, config.reachDistance, chunkManager);
             if (res.hit)
                renderer.drawHighlightBlock(res.pos, cam.viewProjection, menuSettings.exposure);
        );

        TIME(metrics, "Update Entities", updateEntities(entities, metrics.deltaTime, chunkPos, chunkManager););
        TIME(metrics, "Draw Entities", ({
            for (auto& e : entities)
                renderer.drawEntity(e.model, e.physics.box.pos, cam.viewProjection, menuSettings.exposure);
        }));

        if (debugMode)
        {
            renderer.drawAxes(cam);
            renderer.drawDebugMenu(metrics, menuSettings, cam.position, config);
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
    const float forwardInput = 1.0f * window.isKeyDown(GLFW_KEY_W) - 1.0f * window.isKeyDown(GLFW_KEY_S);
    const float rightInput   = 1.0f * window.isKeyDown(GLFW_KEY_D) - 1.0f * window.isKeyDown(GLFW_KEY_A);
    const float upInput      = 1.0f * window.isKeyDown(GLFW_KEY_SPACE) - 1.0f * window.isKeyDown(GLFW_KEY_LEFT_SHIFT);

    if (forwardInput == 0 && rightInput == 0 && upInput == 0)
        return glm::vec3(0.0f);

    const glm::vec3 forward = glm::normalize(glm::vec3(lookDir.x, 0.0f, lookDir.z));
    assert(!std::isnan(forward.x) && !std::isnan(forward.y) && !std::isnan(forward.z));
    const glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

    input += forward * forwardInput;
    input += right * rightInput;
    input.y += upInput;

    assert(std::isnan(input.x) == false && std::isnan(input.y) == false && std::isnan(input.z) == false);

    return glm::normalize(input);
}

void placeBlock(ChunkManager& chunkManager, Camera& cam, BLOCK_TYPE block, SQLite::Database& db, float reachDistance)
{
    if (block == BLOCK_TYPE::INVALID)
    {
        LOG_WARN("Cannot place block of type INVALID");
        return;
    }

    RaycastResult res = raycast(cam.position - cam.lookDir, cam.lookDir, reachDistance, chunkManager);
    if (!res.hit)
        return;

    const auto positionInChunk = worldPosToChunkBlockPos(res.pos);

    const glm::ivec3& chunkPos = res.chunk->chunkPosition;

    std::string stmt;
    if (block == BLOCK_TYPE::AIR)
    {
        res.chunk->setBlockUnsafe(positionInChunk, BLOCK_TYPE::AIR);
        saveBlockChanges(db, chunkPos, positionInChunk, BLOCK_TYPE::AIR);
    }
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
        {
            res.chunk->setBlockUnsafe(neighbourBlockPos, block);
            saveBlockChanges(db, chunkPos, neighbourBlockPos, block);
        }
        else
        {
            glm::uvec3 blockPosInOtherChunk;
            blockPosInOtherChunk.x = (neighbourBlockPos.x % Chunk::CHUNK_SIZE + Chunk::CHUNK_SIZE) % Chunk::CHUNK_SIZE;
            blockPosInOtherChunk.y = (neighbourBlockPos.y % Chunk::CHUNK_SIZE + Chunk::CHUNK_SIZE) % Chunk::CHUNK_SIZE;
            blockPosInOtherChunk.z = (neighbourBlockPos.z % Chunk::CHUNK_SIZE + Chunk::CHUNK_SIZE) % Chunk::CHUNK_SIZE;

            Chunk* neighbourChunk = chunkManager.getChunk(chunkPos + offset);
            assert(neighbourChunk != nullptr);
            assert(neighbourChunk->getBlockSafe(blockPosInOtherChunk) != BLOCK_TYPE::INVALID);

            neighbourChunk->setBlockUnsafe(blockPosInOtherChunk, block);
            saveBlockChanges(db, neighbourChunk->chunkPosition, neighbourBlockPos, block);
        }
    }

    db.exec(stmt);

    if (positionInChunk.x == 0)
    {
        Chunk* chunk = chunkManager.getChunk({chunkPos.x - 1, chunkPos.y, chunkPos.z});
        if (chunk) chunk->isMeshBaked = false;
    }
    else if (positionInChunk.x == Chunk::CHUNK_SIZE - 1)
    {
        Chunk* chunk = chunkManager.getChunk({chunkPos.x + 1, chunkPos.y, chunkPos.z});
        if (chunk) chunk->isMeshBaked = false;
    }
    if (positionInChunk.y == 0)
    {
        Chunk* chunk = chunkManager.getChunk({chunkPos.x, chunkPos.y - 1, chunkPos.z});
        if (chunk) chunk->isMeshBaked = false;
    }
    else if (positionInChunk.y == Chunk::CHUNK_SIZE - 1)
    {
        Chunk* chunk = chunkManager.getChunk({chunkPos.x, chunkPos.y + 1, chunkPos.z});
        if (chunk) chunk->isMeshBaked = false;
    }
    if (positionInChunk.z == 0)
    {
        Chunk* chunk = chunkManager.getChunk({chunkPos.x, chunkPos.y, chunkPos.z - 1});
        if (chunk) chunk->isMeshBaked = false;
    }
    else if (positionInChunk.z == Chunk::CHUNK_SIZE - 1)
    {
        Chunk* chunk = chunkManager.getChunk({chunkPos.x, chunkPos.y, chunkPos.z + 1});
        if (chunk) chunk->isMeshBaked = false;
    }
}
