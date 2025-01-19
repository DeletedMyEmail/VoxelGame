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

void clear(const Window& window, glm::vec4 color = {0.07f, 0.14f, 0.17f, 1.0f}) ;
void drawChunk(Chunk& chunk, const Shader& shader, const Texture& texture, const Window& window, const Camera& cam);
void drawAxes(const Window& window, const Shader& shader, const Camera& cam);
void drawPlayer(glm::vec3 position, const Window& window, const Shader& shader, const Camera& cam);

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
    0b1000000001u, // Top-right
    0b1001000011u, // Bottom-right
    0b1001001111u, // Bottom-left
    0b1000001101u, // Top-left
    // Back Face
    0b1011001110u, // Bottom-left
    0b1011010010u, // Bottom-right
    0b1010010000u, // Top-right
    0b1010001100u, // Top-left
    // Left Face
    0b0101010111u, // Bottom-left
    0b0101011110u, // Bottom-right
    0b0100011100u, // Top-right
    0b0100010101u, // Top-left
    // Right Face
    0b0110011000u, // Top-right
    0b0111011010u, // Bottom-right
    0b0111100011u, // Bottom-left
    0b0110100001u, // Top-left
    // Top Face
    0b0000100000u, // Top-right
    0b0001100001u, // Bottom-right
    0b0001101101u, // Bottom-left
    0b0000101100u, // Top-left
    // Bottom Face
    0b0011101111u, // Bottom-left
    0b0011110011u, // Bottom-right
    0b0010110010u, // Top-right
    0b0010101110u, // Top-left
};

