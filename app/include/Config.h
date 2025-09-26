#pragma once

#include "Block.h"

struct MenuSettings
{
    BLOCK_TYPE selectedBlock;
    float camSpeed, exposure;
    bool playerPhysicsOn;
};

struct GameConfig
{
    std::string saveGamePath = "world.db";
    uint32_t renderDistance = 10;
    uint32_t loadDistance = 12;
    uint32_t maxLoadsPerFrame = 32;
    uint32_t maxUnloadsPerFrame = maxLoadsPerFrame;
    uint32_t threadCount = std::thread::hardware_concurrency();
    uint32_t maxBakesPerFrame = threadCount - 1;
    uint32_t worldSeed = std::chrono::steady_clock::now().time_since_epoch().count();
    float reachDistance = 16.0f;
};

inline GameConfig gameConfig;

bool loadConfig(const char* path, GameConfig& config);