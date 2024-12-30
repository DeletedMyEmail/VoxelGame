#include "Noise.h"
#include <cmath>

#include "Log.h"
#include "glm/ext/quaternion_geometric.hpp"

// credit: https://www.youtube.com/watch?v=kCIaHqb60Cw

float cubicInterpolation(const float x1, const float x2, const float weight)
{
    return (x2 - x1) * (3.0 - weight * 2.0) * weight * weight + x1;
}

glm::vec2 randomGradient(const glm::ivec2 gridPos, const unsigned int seed)
{
    constexpr unsigned int bits = 8 * sizeof(unsigned int);
    constexpr unsigned int bitsHalf = bits / 2;

    unsigned int a = gridPos.x + seed;
    unsigned int b = gridPos.y + seed;
    a *= 3284157443;

    b ^= a << bitsHalf | a >> bits - bitsHalf;
    b *= 1911520717;

    a ^= b << bitsHalf | b >> bits - bitsHalf;
    a *= 2048419325;

    const float random = a * (M_PI / ~(~0u >> 1)); // in [0, 2*Pi]

    return {std::sin(random), std::cos(random)};
}

float dotGridGradient(const glm::ivec2 gridPos, const glm::vec2 pos, const unsigned int seed)
{
    const glm::vec2 gradient = randomGradient(gridPos, seed);
    const glm::vec2 distance = pos - glm::vec2(gridPos);

    return glm::dot(distance, gradient);
}

float perlin(const glm::vec2 pos, const unsigned int seed)
{
    const glm::ivec2 gridPos = glm::ivec2(pos);
    const glm::vec2 weights = pos - glm::vec2(gridPos);

    // top corners
    float dot0 = dotGridGradient(gridPos, pos, seed);
    float dot1 = dotGridGradient(gridPos + glm::ivec2(1,0), pos, seed);
    const float ci1 = cubicInterpolation(dot0, dot1, weights.x);

    // bottom corners
    dot0 = dotGridGradient(gridPos + glm::ivec2(0, 1), pos, seed);
    dot1 = dotGridGradient(gridPos + glm::ivec2(1), pos, seed);
    const float ci2 = cubicInterpolation(dot0, dot1, weights.x);

    return cubicInterpolation(ci1, ci2, weights.y);
}

unsigned char** genPerlinMap(const unsigned int width, const unsigned int height, unsigned char min, unsigned char max, const unsigned int seed)
{
    auto** image = new unsigned char*[height];
    for (unsigned int y = 0; y < height; y++)
        image[y] = new unsigned char[width];

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const glm::vec2 pos = glm::vec2(x, y) / glm::vec2(width, height) * 10.0f;

            const float noiseValue = glm::clamp(perlin(pos, seed), -1.0f, 1.0f);
            const auto color = glm::mix(min, max, (noiseValue + 1.0f) / 2.0f);

            image[y][x] = color;
        }
    }

    return image;
}

void freePerlinMap(unsigned char** image, const unsigned int height) {
    for (unsigned int y = 0; y < height; y++)
        delete[] image[y];

    delete[] image;
}