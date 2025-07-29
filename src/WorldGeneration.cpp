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
    noise.SetFrequency(0.01f);        // Controls terrain scale
    noise.SetFractalOctaves(4);       // Number of noise layers
    noise.SetFractalLacunarity(2.0f); // Frequency multiplier between octaves
    noise.SetFractalGain(0.5f);       // Amplitude multiplier between octaves

    return noise;
}

FastNoiseLite genTreeNoise()
{
    FastNoiseLite noise;
    noise.SetSeed(config::WORLD_SEED);

    return noise;
}

inline static float AMP = 2000.0f;
inline static constexpr uint32_t MAX_GEN_HEIGHT = 30;// Chunk::CHUNK_SIZE - Chunk::CHUNK_SIZE / 4;
inline static constexpr uint32_t MIN_GEN_HEIGHT = 1;//Chunk::CHUNK_SIZE / 4;

uint32_t mapToHeightRange(const float value)
{
    assert(value == glm::clamp(value, -AMP, AMP));

    const float normalized = (value + AMP) / (2.0f * AMP);
    return uint32_t(MIN_GEN_HEIGHT + normalized * (MAX_GEN_HEIGHT - MIN_GEN_HEIGHT));
}

uint32_t noiseToHeight(const float value)
{
    const float normalized = (value + 1.0f) * 0.5f;
    return MIN_GEN_HEIGHT + normalized * (MAX_GEN_HEIGHT - MIN_GEN_HEIGHT);
}

uint32_t getHeightAt(const glm::ivec2& pos)
{
    static FastNoiseLite terrainNoise = genTerrainNoise();

    float val = terrainNoise.GetNoise((float) pos.x, (float) pos.y);
    //val = glm::pow(val,4.0f) * AMP;

    return noiseToHeight(val);
}
