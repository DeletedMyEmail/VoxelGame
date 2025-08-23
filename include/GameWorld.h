#pragma once

#include "Block.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "SQLiteCpp/Backup.h"

SQLite::Database initDB();
void saveBlockChanges(SQLite::Database& db, const glm::ivec3& chunkPos, const glm::ivec3& positionInChunk, BLOCK_TYPE blockType);

uint32_t getHeightAt(const glm::ivec2& pos);
bool hasTree(const glm::ivec2& pos);
bool isForest(const glm::ivec2& pos);

inline const uint32_t WORLD_HEIGHT = 6;
inline const uint32_t SEA_LEVEL = 64.0f;
inline const float MAX_HEIGHT = 300.0f;
inline const float MIN_HEIGHT = 10.0f;