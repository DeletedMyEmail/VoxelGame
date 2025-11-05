#include "GameLayer.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include <cstmlib/Profiling.h>
#include "imgui.h"
#include "Camera.h"
#include "Chunk.h"
#include "Window.h"
#include "OpenGLHelper.h"
#include "Raycast.h"
#include "Entity.h"
#include "Rendering.h"
#include "GameWorld.h"
#include "SQLiteCpp/Database.h"
#include "stb/stb_image.h"
#include <libconfig.h++>
#include "Application.h"
#include "Config.h"

static glm::vec3 moveInput(const Window& window, const glm::vec3& lookDir);
static void placeBlock(ChunkManager& chunkManager, Camera& cam, BLOCK_TYPE block, SQLite::Database& db, float reachDistance);
static glm::vec3 moveInput(const Window& window, const glm::vec3& lookDir);

#define m_Window core::Application::get().getWindow()

GameLayer::GameLayer(const std::string& name)
    : Layer(name),
      m_Renderer(m_Window.getHandle()),
      m_Cam(glm::vec3{0, WorldGenerationData::MAX_HEIGHT + 2, 0}, 90.0f, m_Window.getSettings().width,
            m_Window.getSettings().height, 0.1f, gameConfig.renderDistance * Chunk::CHUNK_SIZE * 4),
      m_ChunkManager(gameConfig),
      m_MenuSettings{BLOCK_TYPE::INVALID, 50.0f, 0.8f, true},
      m_Database(initDB(gameConfig.saveGamePath)),
      m_PlayerPhysics(BoundingBox{m_Cam.position, glm::vec3{1, 2, 1}}, glm::vec3(0.0f)),
      m_PrevCursorPos(m_Window.getMousePosition())
{
    m_Window.disableCursor();

    const EntityBehavior noBehavior;
    m_EntityManager.addEntity(BoundingBox{
                                  glm::vec3{10.0f, WorldGenerationData::MAX_HEIGHT - 4, 10.0f},
                                  glm::vec3{1.0f, 1.0f, 1.0f}
                              },
                              createEntityWireframe(glm::vec3{1.0f, 2.0f, 1.0f}),
                              100.0f,
                              noBehavior);
    m_EntityManager.addEntity(BoundingBox{
                                  glm::vec3{10.0f, WorldGenerationData::MAX_HEIGHT - 4, 11.0f},
                                  glm::vec3{1.0f, 2.0f, 1.0f}
                              },
                              createEntityWireframe(glm::vec3{1.0f, 2.0f, 1.0f}),
                              100.0f,
                              noBehavior);
    m_EntityManager.addEntity(BoundingBox{
                                  glm::vec3{11.0f, WorldGenerationData::MAX_HEIGHT - 4, 10.0f},
                                  glm::vec3{1.0f, 3.0f, 1.0f}
                              },
                              createEntityWireframe(glm::vec3{1.0f, 2.0f, 1.0f}),
                              100.0f,
                              noBehavior);
    m_EntityManager.addEntity(BoundingBox{
                                  glm::vec3{11.0f, WorldGenerationData::MAX_HEIGHT - 4, 11.0f},
                                  glm::vec3{1.0f, 4.0f, 1.0f}
                              },
                              createEntityWireframe(glm::vec3{1.0f, 2.0f, 1.0f}),
                              100.0f,
                              noBehavior);
}

void GameLayer::onUpdate(const double dt)
{
    m_Metrics.update(dt);

    const auto chunkPos = worldPosToChunkPos(m_PlayerPhysics.box.pos);

    PROFILE(m_Metrics, "Update Entities", m_EntityManager.updateEntities(dt, m_ChunkManager));
    PROFILE(m_Metrics, "Chunk Unloading", m_ChunkManager.unloadChunks(chunkPos));
    PROFILE(m_Metrics, "Chunk Loading", m_ChunkManager.loadChunks(chunkPos, m_Database));
    PROFILE(m_Metrics, "Chunk Baking", m_ChunkManager.bakeChunks(chunkPos));

    glm::vec3 in = moveInput(m_Window, m_Cam.lookDir);
    if (m_MenuSettings.playerPhysicsOn)
    {
        if (m_PlayerGrounded)
        {
            in.y = 0.0f;
            m_PlayerPhysics.velocity += in * (float) dt;
        }
        applyGravity(m_PlayerPhysics, (float) dt);
        if (applyVelocityAndHandleCollisions(m_ChunkManager, m_PlayerPhysics)) m_PlayerGrounded = true;
        const glm::vec3 off = m_PlayerPhysics.box.pos - m_Cam.position + glm::vec3(0.5f, 1.5f, 0.5f);
        m_Cam.move(off);
    }
    else
    {
        const glm::vec3 vel = in * (float) dt * m_MenuSettings.camSpeed;
        m_PlayerPhysics.box.pos += vel;
        m_PlayerPhysics.velocity = glm::vec3(0);
        m_Cam.move(vel);
    }
    m_Cam.updateView();
}

void GameLayer::onRender()
{
    const float skyExposure = 0.5f + 0.5f * m_MenuSettings.exposure;

    m_Renderer.clearFrame(skyExposure, m_DebugMode);
    PROFILE(m_Metrics, "Chunk Drawing", m_ChunkManager.drawChunks(m_Renderer, m_Cam.viewProjection, m_MenuSettings.exposure));
    PROFILE(m_Metrics, "Block Highlighting",
         const RaycastResult res = raycast(m_Cam.position - m_Cam.lookDir, m_Cam.lookDir, gameConfig.reachDistance, m_ChunkManager);
         if (res.hit)
            m_Renderer.drawHighlightBlock(res.pos, m_Cam.viewProjection, m_MenuSettings.exposure);
    );
    PROFILE(m_Metrics, "Draw Entities", m_EntityManager.drawEntities(m_Renderer, m_Cam.viewProjection, m_MenuSettings.exposure));

    if (m_DebugMode)
    {
        m_Renderer.drawAxes(m_Cam);
        m_Renderer.drawDebugMenu(m_Metrics, m_MenuSettings, m_Cam.position, gameConfig);
    }

    //static auto entityModel = createEntityWireframe(m_PlayerPhysics.box.size);
    //m_Renderer.drawEntity(entityModel, m_PlayerPhysics.box.pos, m_Cam.viewProjection, m_MenuSettings.exposure);
}

void GameLayer::keyPressCallback(const core::Event& e)
{
    switch (e.keyEvent.key)
    {
        case GLFW_KEY_ESCAPE: core::Application::get().stop(); break;
        case GLFW_KEY_F3: m_DebugMode = !m_DebugMode; break;
        case GLFW_KEY_F5: m_ChunkManager.dropChunkMeshes(); break;
        case GLFW_KEY_V: m_CursorLocked = !m_CursorLocked; m_Window.disableCursor(m_CursorLocked); break;
        case GLFW_KEY_SPACE: if (m_MenuSettings.playerPhysicsOn && m_PlayerGrounded) {m_PlayerPhysics.velocity.y += 0.2; m_PlayerGrounded = false;} break;
        default: break;
    }
}

void GameLayer::cursorMoveCallback(const core::Event& e)
{
    ImGui::GetIO().MousePos = ImVec2(float(e.cursorEvent.pos.x), float(e.cursorEvent.pos.y));
    const glm::dvec2 offset = e.cursorEvent.pos - m_PrevCursorPos;
    if (m_CursorLocked)
        m_Cam.rotate({offset.x, offset.y});
    m_PrevCursorPos = e.cursorEvent.pos;
}

void GameLayer::mouseReleasedCallback(const core::Event& e)
{
    ImGui::GetIO().MouseDown[e.mouseEvent.button] = false;
}

void GameLayer::mousePressedCallback(core::Event& e)
{
    ImGui::GetIO().MouseDown[e.mouseEvent.button] = true;
    if (!m_CursorLocked)
        return;

    if (e.mouseEvent.button == GLFW_MOUSE_BUTTON_LEFT)
        placeBlock(m_ChunkManager, m_Cam, m_MenuSettings.selectedBlock, m_Database, gameConfig.reachDistance);
}

void GameLayer::onEvent(core::Event& e)
{
    switch (e.type)
    {
        case core::EventType::KeyPressed: keyPressCallback(e); break;
        case core::EventType::CursorMoved: cursorMoveCallback(e); break;
        case core::EventType::MouseButtonPressed: mousePressedCallback(e); break;
        case core::EventType::MouseButtonReleased: mouseReleasedCallback(e); break;
        case core::EventType::WindowClose: core::Application::get().stop(); break;
        default: break;
    }
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