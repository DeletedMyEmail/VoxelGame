#pragma once

#include "Block.h"

struct MenuSettings
{
    BLOCK_TYPE selectedBlock;
    float camSpeed, exposure;
    bool collisionsOn;
};

struct ProgramConfig
{
    std::string saveGamePath = "world.db";
    uint32_t renderDistance = 10;
    uint32_t loadDistance = renderDistance * 1.5;
    uint32_t maxLoadsPerFrame = 32;
    uint32_t maxUnloadsPerFrame = maxLoadsPerFrame;
    uint32_t threadCount = std::thread::hardware_concurrency();
    uint32_t maxBakesPerFrame = threadCount - 1;
    uint32_t worldSeed = 42;
    float reachDistance = 16.0f;
};

bool loadConfig(const char* path, ProgramConfig& config);