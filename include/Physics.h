#pragma once

#include "Chunk.h"
#include "VertexArray.h"
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

constexpr float GRAVITY = -9.81f;
constexpr float TERMINAL_VELOCITY = -50.0f;

void updateEntities(std::vector<Entity> entities, float dtime, const glm::ivec3& pos);

void handleCollision(ChunkManager& chunkManager, PhysicsObject& obj);
BoundingBox getBroadphaseBox(const PhysicsObject& obj);
bool isColliding(const BoundingBox& a, const BoundingBox& b);
CollisionData getCollision(PhysicsObject& a, BoundingBox& b);
void resolveCollision(PhysicsObject& a, const CollisionData& collisionData);
