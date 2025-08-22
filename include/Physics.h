#pragma once
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

BoundingBox getBroadphaseBox(const PhysicsObject& obj);
bool isColliding(const BoundingBox& a, const BoundingBox& b);
CollisionData getCollision(PhysicsObject& a, BoundingBox& b);
void resolveCollision(PhysicsObject& a, BoundingBox& b, const CollisionData& collisionData);