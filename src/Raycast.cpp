#include <glm/glm.hpp>
#include <cmath>
#include <functional>
#include <limits>
#include "Block.h"
#include "Raycast.h"
#include <stdexcept>

static float mod(float value, float modulus) { return std::fmod(std::fmod(value, modulus) + modulus, modulus); }

static float intbound(float s, float ds)
{
    if (ds < 0)
        return intbound(-s, -ds);

    s = mod(s, 1.0f);
    return (1.0f - s) / ds;
}

static int signum(const float x) { return (x > 0) - (x < 0); }

RaycastResult raycast(const glm::vec3& origin, const glm::vec3& dir, const float radius, std::unordered_map<uint64_t, Chunk*>& chunks)
{
    glm::ivec3 blockPos{std::floor(origin.x), std::floor(origin.y), std::floor(origin.z)};
    int stepX = signum(dir.x), stepY = signum(dir.y), stepZ = signum(dir.z);
    float tMaxX = (dir.x != 0) ? intbound(origin.x, dir.x) : std::numeric_limits<float>::infinity();
    float tMaxY = (dir.y != 0) ? intbound(origin.y, dir.y) : std::numeric_limits<float>::infinity();
    float tMaxZ = (dir.z != 0) ? intbound(origin.z, dir.z) : std::numeric_limits<float>::infinity();
    float tDeltaX = (dir.x != 0) ? std::abs(1.0f / dir.x) : std::numeric_limits<float>::infinity();
    float tDeltaY = (dir.y != 0) ? std::abs(1.0f / dir.y) : std::numeric_limits<float>::infinity();
    float tDeltaZ = (dir.z != 0) ? std::abs(1.0f / dir.z) : std::numeric_limits<float>::infinity();
    FACE face = INVALID;

    assert(dir.x != 0 || dir.y != 0 || dir.z != 0);

    float length = std::sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
    float maxT = radius / length;

    while (blockPos.y < Chunk::MAX_HEIGHT)
    {
        if (!(blockPos.x < 0 || blockPos.y < 0 || blockPos.z < 0 || blockPos.y >= Chunk::MAX_HEIGHT))
        {
            glm::uvec2 chunkPos = worldPosToChunkPos(blockPos);
            auto it = chunks.find(packChunkPos(chunkPos.x, chunkPos.y));
            assert(it != chunks.end());
            Chunk* chunk = it->second;
            const BLOCK_TYPE block = chunk->getBlockUnsafe(worldPosToChunkBlockPos(blockPos));
            assert(block != BLOCK_TYPE::INVALID);
            if (block != BLOCK_TYPE::AIR)
               return {blockPos, chunk, block, face, face != INVALID};
        }

        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) {
                if (tMaxX > maxT) break;
                blockPos.x += stepX;
                tMaxX += tDeltaX;
                face = (stepX > 0 ? FACE::LEFT : FACE::RIGHT);
            } else {
                if (tMaxZ > maxT) break;
                blockPos.z += stepZ;
                tMaxZ += tDeltaZ;
                face = (stepZ > 0 ? FACE::BACK : FACE::FRONT);
            }
        } else {
            if (tMaxY < tMaxZ) {
                if (tMaxY > maxT) break;
                blockPos.y += stepY;
                tMaxY += tDeltaY;
                face = (stepY > 0 ? FACE::BOTTOM : FACE::TOP);
            } else {
                if (tMaxZ > maxT) break;
                blockPos.z += stepZ;
                tMaxZ += tDeltaZ;
                face = (stepZ > 0 ? FACE::BACK : FACE::FRONT);
            }
        }
    }

    return {blockPos, nullptr, BLOCK_TYPE::INVALID, FACE::INVALID, false};
}