#pragma once

#include "Camera.h"
#include "Chunk.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"

class App {
public:
    App();
    ~App() = default;

    void run();
private:
    void processCamInputs(glm::dvec2& prevMousePos, float deltaTime);
    void initChunks(unsigned int chunksPerSide);
private:
    Window m_Window;
    Texture m_TextureAtlas;
    Shader m_Shader;
    Camera m_Camera;
    std::vector<Chunk> m_Chunks;
};
