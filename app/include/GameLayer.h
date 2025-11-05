#pragma once
#include "Chunk.h"
#include "Layer.h"
#include "Entity.h"
#include "Rendering.h"

class GameLayer final : public core::Layer
{
public:
    GameLayer(const std::string& name = "GameLayer");
    ~GameLayer() override = default;
    void onUpdate(double dt) override;
    void onRender() override;
    void onEvent(core::Event& e) override;
private:
    void keyPressCallback(const core::Event& e);
    void cursorMoveCallback(const core::Event& e);
    void mousePressedCallback(core::Event& e);
    void mouseReleasedCallback(const core::Event& e);
private:
    //Window& m_Window;
    Renderer m_Renderer;
    Camera m_Cam;
    ChunkManager m_ChunkManager;
    MenuSettings m_MenuSettings;
    SQLite::Database m_Database;
    EntityManager m_EntityManager;
    PhysicsObject m_PlayerPhysics;
    bool m_PlayerGrounded = false;
    Metrics m_Metrics;
    glm::dvec2 m_PrevCursorPos;
    bool m_DebugMode = true;
    bool m_CursorLocked = true;
};
