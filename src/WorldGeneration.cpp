#include "WorldGeneration.h"
#include "Chunk.h"
#include "Config.h"
#include "glm/common.hpp"
#include "glm/exponential.hpp"
#include "FastNoiseLite.h"

FastNoiseLite genTerrainNoise()
{
    FastNoiseLite noise;
    noise.SetSeed(config::WORLD_SEED);
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetFractalOctaves(3);
    noise.SetFrequency(0.005f);
    noise.SetFractalGain(0.7f);
    noise.SetFractalWeightedStrength(-0.5f);

    return noise;
}

FastNoiseLite genTerrainNoise2()
{
    FastNoiseLite noise;
    noise.SetSeed(config::WORLD_SEED);
    noise.SetNoiseType(FastNoiseLite::NoiseType_ValueCubic);
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetFrequency(0.08f);

    return noise;
}

FastNoiseLite genTreeNoise()
{
    FastNoiseLite noise;
    noise.SetSeed(config::WORLD_SEED);

    return noise;
}

uint32_t noiseToHeight(float value)
{
    constexpr uint32_t MAX_GEN_HEIGHT = Chunk::CHUNK_SIZE * WORLD_HEIGHT;
    constexpr uint32_t MIN_GEN_HEIGHT = MAX_GEN_HEIGHT / 3;

    value = (value + 1.0f) / 2.0f;
    value = glm::pow(value, 4.8f);
    assert(value < 1.0f);
    return value * (MAX_GEN_HEIGHT - MIN_GEN_HEIGHT) + MIN_GEN_HEIGHT;
}

uint32_t getHeightAt(const glm::ivec2& pos)
{
    static FastNoiseLite terrainNoise1 = genTerrainNoise();
    static FastNoiseLite terrainNoise2 = genTerrainNoise2();

    float val = terrainNoise1.GetNoise((float) pos.x, (float) pos.y);
    val += terrainNoise2.GetNoise((float) pos.x, (float) pos.y);
    //val /= 2.0f;

    const uint32_t height = noiseToHeight(val);
    assert(height < MAX_GEN_HEIGHT);
    return height;
}
