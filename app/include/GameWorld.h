#pragma once

#include "Block.h"
#include "FastNoiseLite.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "SQLiteCpp/Database.h"

SQLite::Database initDB(const std::string& dbPath);
void saveBlockChanges(SQLite::Database& db, const glm::ivec3& chunkPos, const glm::ivec3& positionInChunk, BLOCK_TYPE blockType);

struct BlockChange
{
    glm::ivec3 positionInChunk;
    BLOCK_TYPE blockType;
};
std::vector<BlockChange> getBlockChangesForChunk(SQLite::Database& db, const glm::ivec3& chunkPos);

struct WorldGenerationData
{
    WorldGenerationData(uint32_t seed);
    uint32_t getHeightAt(const glm::ivec2& pos) const;
    bool hasTree(const glm::ivec2& pos) const;
    bool isForest(const glm::ivec2& pos) const;

    static constexpr uint32_t WORLD_HEIGHT = 6;
    static constexpr uint32_t SEA_LEVEL = 64.0f;
    static constexpr float MAX_HEIGHT = 300;
    static constexpr float MIN_HEIGHT = 10.0f;

    FastNoiseLite treeNoise, forestNoise, primaryNoise, secondaryNoise, biomeNoise;
};