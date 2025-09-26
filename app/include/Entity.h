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

/*struct Entity
{
    Entity(const glm::vec3& pos, const glm::vec3& size, float health = 100.0f, float speed = 5.0f);

    VertexArray model;
    PhysicsObject physics;
    float health, speed;
};*/

struct CollisionData
{
    glm::vec3 normal;
    float entryTime;
};
void handleCollision(ChunkManager& chunkManager, PhysicsObject& obj);
void applyGravity(PhysicsObject& obj, float dtime);

struct EntityBehavior
{
    std::function<void(PhysicsObject&)> movementFunction;
    std::function<void(CollisionData&)> collisionFunction;
};

struct EntityManager
{
    EntityManager() = default;
    ~EntityManager() = default;

    void loadEntities();
    void unloadEntities();
    void updateEntities(double dt, ChunkManager& chunkManager);
    void drawEntities(const Renderer& renderer, const glm::mat4& viewProjection, float exposure) const;
    void addEntity(const BoundingBox& box, VertexArray&& model, float health, const EntityBehavior& behavior);
    void removeEntity(size_t index);
    size_t count() const;

    // Entity Component System
    std::vector<PhysicsObject> m_PhysicsObjects;
    std::vector<VertexArray> m_Models;
    std::vector<EntityBehavior> m_BehaviorComponents; // not used yet
    std::vector<float> m_Healths;
};

VertexArray createEntityWireframe(const glm::vec3& size);

constexpr float GRAVITY = -1.0f;
constexpr float TERMINAL_VELOCITY = -2.0f;

