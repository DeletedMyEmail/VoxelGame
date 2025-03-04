#pragma once

#include "Camera.h"
#include "Chunk.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"

#ifndef RELEASE
    #define GLCall(x) x; checkOpenGLErrors();
#else
    #define GLCall(x) x;
#endif

void checkOpenGLErrors();

enum DRAW_GEOMETRY
{
    LINES = GL_LINES,
    TRIANGLES = GL_TRIANGLES,
};

class Renderer
{
public:
    Renderer(int width, int height);
    ~Renderer();

    void update(const Window& window, const glm::vec4& color = {0.07f, 0.14f, 0.17f, 1.0f}) const;
    void drawChunk(Chunk& chunk, const Camera& cam) const;
    void draw(const VertexArray& vao, DRAW_GEOMETRY geo, const glm::vec3& position, const Camera& cam) const;
    void draw(const char* text, const glm::ivec2& position, float scale) const;
private:
    Texture m_TextureAtlas;
    Shader m_ChunkShader, m_BasicShader;
};

constexpr unsigned int FRONT_FACE_INDEX = 0;
constexpr unsigned int BACK_FACE_INDEX = 1;
constexpr unsigned int LEFT_FACE_INDEX = 2;
constexpr unsigned int RIGHT_FACE_INDEX = 3;
constexpr unsigned int TOP_FACE_INDEX = 4;
constexpr unsigned int BOTTOM_FACE_INDEX = 5;

// 32 bits - 0: pos-z, 1: pos-y, 2: pos-x, 3-5: texture-x, 6: texture-y, 7-9: normal
// (missing: 10: atlas-x, 11-14: atlas-y, 15-18: pos-rel-to-chunk-x, 19-26: pos-rel-to-chunk-y, 27-30: pos-rel-to-chunk-z, 31: unused)
constexpr GLuint BLOCK_FACES[] = {
    // Front Face
    0b1000000110u, // Top-right
    0b1001000100u, // Bottom-right
    0b1001001000u, // Bottom-left
    0b1000001010u, // Top-left
    // Back Face
    0b1011001001u, // Bottom-left
    0b1011010101u, // Bottom-right
    0b1010010111u, // Top-right
    0b1010001011u, // Top-left
    // Left Face
    0b0101010000u, // Bottom-left
    0b0101011001u, // Bottom-right
    0b0100011011u, // Top-right
    0b0100010010u, // Top-left
    // Right Face
    0b0110011111u, // Top-right
    0b0111011101u, // Bottom-right
    0b0111100100u, // Bottom-left
    0b0110100110u, // Top-left
    // Top Face
    0b0000100111u, // Top-right
    0b0001100110u, // Bottom-right
    0b0001101010u, // Bottom-left
    0b0000101011u, // Top-left
    // Bottom Face
    0b0011101000u, // Bottom-left
    0b0011110100u, // Bottom-right
    0b0010110101u, // Top-right
    0b0010101001u, // Top-left
};