#pragma once

#include "Camera.h"
#include "Chunk.h"
#include "GameWorld.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"

class App {
public:
    App();
    ~App() = default;

    void run();
private:
    Window m_Window;
    GameWorld m_GameWorld;
    Texture m_TextureAtlas;
    Shader m_Shader;
    Camera m_Camera;
};
