#pragma once
#include "Chunk.h"
#include "Layer.h"
#include "Entity.h"
#include "Rendering.h"

class GameLayer final : public core::Layer
{
friend class DebugLayer;
public:
    GameLayer(const std::string& name, const GameConfig& config);
    ~GameLayer() override = default;
    void onUpdate(double dt) override;
    void onRender() override;
    bool onEvent(core::Event& e) override;
private:
    bool keyPressCallback(const core::Event& e);
    bool cursorMoveCallback(const core::Event& e);
    bool mousePressedCallback(const core::Event& e);
private:
    //Window& m_Window;
    GameConfig m_GameConfig;
    Renderer m_Renderer;
    Camera m_Cam;
    ChunkManager m_ChunkManager;
    SQLite::Database m_Database;
    EntityManager m_EntityManager;
    PhysicsObject m_PlayerPhysics;
    bool m_PlayerGrounded = false;
    Metrics m_Metrics;
    glm::dvec2 m_PrevCursorPos;
    bool m_CursorLocked = true;
    BLOCK_TYPE selectedBlock;
    float m_CamSpeed, m_Exposure;
    bool m_PlayerPhysicsOn;
};
