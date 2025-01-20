#pragma once

#include <Renderer.h>
#include "Camera.h"
#include "Chunk.h"
#include "GameWorld.h"
#include "Window.h"

class App {
public:
    App();
    ~App() = default;

    void run();
private:
    glm::vec3 getPlayerMoveInputs() const;
    void processHotkeys(int key, int scancode, int mods);
private:
    Window m_Window;
    GameWorld m_GameWorld;
    Renderer m_Renderer;
    Camera m_Camera;
};
