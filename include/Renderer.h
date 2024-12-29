#pragma once

#include "Camera.h"
#include "Chunk.h"
#include "glad/glad.h"
#include "Log.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexArray.h"
#include "Window.h"

#ifndef RELEASE
    #define GLCall(x) x; checkOpenGLErrors();
#else
    #define GLCall(x) x;
#endif

void checkOpenGLErrors();

inline static std::array<GLsizei, 36> indices
{
    0, 1, 2, 2, 3, 0,       // Front face
    4, 5, 6, 6, 7, 4,       // Back face
    8, 9, 10, 10, 11, 8,    // Left face
    12, 13, 14, 14, 15, 12, // Right face
    16, 17, 18, 18, 19, 16, // Top face
    20, 21, 22, 22, 23, 20  // Bottom face
};

class BlockRenderer
{
public:
    BlockRenderer(const char* textureAtlasPath);

    void clear(const Window& window, glm::vec4 color = {0.07f, 0.14f, 0.17f, 1.0f}) const;
    void draw(Chunk& chunk, const Shader&, const Window& window, const Camera& cam);
private:
    void setInstanceData(Chunk& chunk);
private:
    VertexArray m_BlockVertArray{};
    IndexBuffer m_BlockIndexBuffer{indices.data(), indices.size()};
    Texture m_TextureAtlas;
};