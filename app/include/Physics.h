#pragma once

#include "Chunk.h"
#include "../../core/include/VertexArray.h"
#include "glm/vec3.hpp"

struct BoundingBox
{
    glm::vec3 pos, size;
};

struct PhysicsObject
{
    BoundingBox box;
    glm::vec3 velocity;
};

struct CollisionData
{
    glm::vec3 normal;
    float entryTime;
};

struct Entity
{
    Entity(const glm::vec3& pos, const glm::vec3& size, float health = 100.0f, float speed = 5.0f);

    VertexArray model;
    PhysicsObject physics;
    float health, speed;
};

//TODO: ECS ?

constexpr float GRAVITY = -1.0f;
constexpr float TERMINAL_VELOCITY = -10.0f;

void updateEntities(std::vector<Entity>& entities, float dtime, const glm::ivec3& pos, ChunkManager& chunkManager);
void handleCollision(ChunkManager& chunkManager, PhysicsObject& obj);
