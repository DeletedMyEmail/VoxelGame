#include "WorldGeneration.h"

FastNoiseLite createBiomeNoise(const BIOME b, const int32_t seed)
{
    FastNoiseLite noise(seed);
    switch (b)
    {
    case PLAINS:
        noise.SetNoiseType(FastNoiseLite::NoiseType_ValueCubic);
        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFrequency(0.007f);
        break;
    case DESERT:
        noise.SetNoiseType(FastNoiseLite::NoiseType_Value);
        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFrequency(0.005f);
        break;
    case FOREST:
        noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
        noise.SetFractalType(FastNoiseLite::FractalType_None);
        noise.SetFrequency(0.002f);
        break;
    case MOUNTAIN:
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFrequency(0.008f);
        break;
    case HILLS:
        noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
        noise.SetFractalType(FastNoiseLite::FractalType_Ridged);
        noise.SetFrequency(0.007f);
        break;
    }

    return noise;
}

BLOCK_TYPE defaultBiomeBlock(const BIOME b)
{
    switch (b)
    {
    case PLAINS:
        return BLOCK_TYPE::GRASS;
    case DESERT:
        return BLOCK_TYPE::SAND;
    case FOREST:
        return BLOCK_TYPE::MELON;
    case MOUNTAIN:
        return BLOCK_TYPE::STONE;
    case HILLS:
        return BLOCK_TYPE::GRASS;
    default:
        return BLOCK_TYPE::INVALID;
    }

}