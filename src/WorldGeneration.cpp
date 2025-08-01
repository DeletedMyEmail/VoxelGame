#include "WorldGeneration.h"
#include "Chunk.h"
#include "glm/common.hpp"
#include "glm/exponential.hpp"
#include "glm/trigonometric.hpp"
#include "FastNoiseLite.h"

FastNoiseLite genPrimNoise();
FastNoiseLite genSecNoise();
FastNoiseLite genBiomeNoise();
FastNoiseLite genTreeNoise();
uint32_t noiseToHeight(float primaryValue, float secondaryValue, float biomeValue);

uint32_t getHeightAt(const glm::ivec2& pos)
{
    static FastNoiseLite primaryNoise = genPrimNoise();
    static FastNoiseLite secondaryNoise = genSecNoise();
    static FastNoiseLite biomeNoise = genBiomeNoise();

    const float primaryValue = primaryNoise.GetNoise(float(pos.x), float(pos.y));
    const float secondaryValue = secondaryNoise.GetNoise(float(pos.x), float(pos.y));
    const float biomeValue = biomeNoise.GetNoise(float(pos.x), float(pos.y));

    return noiseToHeight(primaryValue, secondaryValue, biomeValue);
}

bool hasTree(const glm::ivec2& pos)
{
    static FastNoiseLite treeNoise = genTreeNoise();
    const float noiseValue = treeNoise.GetNoise(float(pos.x), float(pos.y));
    return noiseValue > 0;
}

uint32_t noiseToHeight(float primaryValue, float secondaryValue, float biomeValue)
{
    // Normalize from [-1, 1] to [0, 1]
    primaryValue = (primaryValue + 1.0f) * 0.5f;
    secondaryValue = (secondaryValue + 1.0f) * 0.5f;
    biomeValue = (biomeValue + 1.0f) * 0.5f;

    // AI slop:

    float valleyHeight = 20.0f + primaryValue * 15.0f + secondaryValue * 5.0f;

    // More interesting plains with rolling hills and small features
    float plainsBase = float(SEA_LEVEL) + primaryValue * 12.0f;
    float plainsDetail = secondaryValue * 8.0f - 4.0f;
    float plainsVariation = glm::sin(primaryValue * 6.28f) * 3.0f + glm::cos(secondaryValue * 6.28f) * 2.0f;
    float plainsHeight = plainsBase + plainsDetail + plainsVariation;

    float mountainHeight = glm::pow(primaryValue, 1.5f);
    float mountainFinal = float(SEA_LEVEL) + mountainHeight * 60.0f + secondaryValue * 15.0f;

    // Rare very tall mountains
    if (primaryValue > 0.85f && secondaryValue > 0.8f) {
        float extremeHeight = (primaryValue - 0.85f) * (secondaryValue - 0.8f) * 2000.0f;
        mountainFinal += extremeHeight;
    }
    else if (secondaryValue > 0.7f) {
        mountainFinal += (secondaryValue - 0.7f) * 60.0f;
    }

    float mesaBase = 80.0f + primaryValue * 30.0f;
    float mesaTop = mesaBase + 40.0f;
    float mesaFinal = (secondaryValue > 0.3f) ?
        mesaTop + secondaryValue * 10.0f :
        mesaBase - (0.3f - secondaryValue) * 20.0f;

    // Ocean/sea areas - flat underwater terrain
    float oceanHeight = float(SEA_LEVEL) - 8.0f + primaryValue * 6.0f + secondaryValue * 3.0f;

    // Adjust biome distribution to include seas
    float finalHeight = 0.0f;

    if (biomeValue <= 0.15f) { // Ocean biome
        finalHeight = oceanHeight;
    }
    else if (biomeValue <= 0.2f) { // Ocean to valley transition
        float blend = (biomeValue - 0.15f) / 0.05f;
        finalHeight = glm::mix(oceanHeight, valleyHeight, blend);
    }
    else if (biomeValue <= 0.35f) { // Valley biome
        if (biomeValue <= 0.3f) {
            finalHeight = valleyHeight;
        } else {
            float blend = (biomeValue - 0.3f) / 0.05f;
            finalHeight = glm::mix(valleyHeight, plainsHeight, blend);
        }
    }
    else if (biomeValue <= 0.7f) { // Plains biome (largest)
        if (biomeValue <= 0.65f) {
            finalHeight = plainsHeight;
        } else {
            float blend = (biomeValue - 0.65f) / 0.05f;
            finalHeight = glm::mix(plainsHeight, mountainFinal, blend);
        }
    }
    else if (biomeValue <= 0.85f) { // Mountain biome
        if (biomeValue <= 0.8f) {
            finalHeight = mountainFinal;
        } else {
            float blend = (biomeValue - 0.8f) / 0.05f;
            finalHeight = glm::mix(mountainFinal, mesaFinal, blend);
        }
    }
    else { // Mesa biome
        if (biomeValue <= 0.95f) {
            finalHeight = mesaFinal;
        } else {
            float blend = (biomeValue - 0.95f) / 0.05f;
            finalHeight = glm::mix(mesaFinal, oceanHeight, blend);
        }
    }

    return uint32_t(glm::clamp(finalHeight, MIN_HEIGHT, MAX_HEIGHT));
}

FastNoiseLite genPrimNoise()
{
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetFrequency(0.003f);
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetFractalOctaves(5);
    noise.SetFractalLacunarity(2.0f);
    noise.SetFractalGain(0.5f);
    noise.SetSeed(config::WORLD_SEED);

    return noise;
}

FastNoiseLite genSecNoise()
{
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetFrequency(0.015f);
    noise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    noise.SetFractalOctaves(3);
    noise.SetFractalLacunarity(2.0f);
    noise.SetFractalGain(0.6f);
    noise.SetSeed(config::WORLD_SEED);

    return noise;
}

FastNoiseLite genBiomeNoise()
{
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetFrequency(0.0008f);
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetFractalOctaves(2);
    noise.SetSeed(config::WORLD_SEED);

    return noise;
}

FastNoiseLite genTreeNoise()
{
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    noise.SetSeed(config::WORLD_SEED);
    noise.SetFrequency(0.9f);

    return noise;
}