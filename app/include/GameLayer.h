#pragma once
#include "Chunk.h"
#include "Layer.h"
#include "Physics.h"
#include "Rendering.h"

class GameLayer : public core::Layer
{
public:
    GameLayer();
    void onUpdate(float dt) override;
    void onRender() override;
    void onEvent(core::Event& e) override;
    void onAttach() override;
    void onDetach() override;
private:
    void keyPressCallback(core::Event& e);
    void cursorMoveCallback(core::Event& e);
    void mousePressedCallback(core::Event& e);
    void mouseReleasedCallback(core::Event& e);
private:
    //Window& m_Window;
    Renderer m_Renderer;
    Camera m_Cam;
    ChunkManager m_ChunkManager;
    MenuSettings m_MenuSettings;
    SQLite::Database m_Database;
    std::vector<Entity> m_Entities;
    Metrics m_Metrics;
    glm::dvec2 m_PrevCursorPos;
    bool m_DebugMode = true;
    bool m_CursorLocked = true;
};
