#pragma once

#include "glm/vec2.hpp"

float cubicInterpolation(float x1, float x2, float weight);
glm::vec2 randomGradient(glm::ivec2 gridPos, unsigned int seed);
float dotGridGradient(glm::ivec2 gridPos, glm::vec2 pos, unsigned int seed);
float perlin(glm::vec2 pos, unsigned int seed);
unsigned char** genPerlinMap(unsigned int width, unsigned int height, unsigned char min, unsigned char max, unsigned int seed, unsigned int octaves = 4, unsigned int lacunarity = 2, float persistance = 0.5f);
void freeMap(unsigned char** image, unsigned int height);