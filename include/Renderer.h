#pragma once

#include "Camera.h"
#include "Chunk.h"
#include "glad/glad.h"
#include "Log.h"
#include "Mesh.h"
#include "Shader.h"
#include "Window.h"

#ifndef RELEASE
    #define GLCall(x) x; checkOpenGLErrors();
#else
    #define GLCall(x) x;
#endif

void checkOpenGLErrors();

class BlockRenderer
{
public:
    BlockRenderer(const char* textureAtlasPath);

    void clear(const Window& window, glm::vec4 color = {0.07f, 0.14f, 0.17f, 1.0f}) const;
    void draw(const Chunk& chunk, const Shader&, const Window& window, const Camera& cam) const;
private:
    Mesh m_BlockMesh;
    Texture m_TextureAtlas;
};

