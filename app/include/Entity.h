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

constexpr float GRAVITY = -1.0f;
constexpr float TERMINAL_VELOCITY = 2.0f;
constexpr float DAMPING = 0.9f;

struct CollisionData
{
    glm::vec3 normal;
    float entryTime;
};
bool move(ChunkManager& chunkManager, PhysicsObject& obj);
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

    void loadEntities(); // TODO: implement
    void unloadEntities(); // TODO: implement
    void updateEntities(double dt, ChunkManager& chunkManager);
    void drawEntities(Renderer& renderer, const glm::mat4& viewProjection, float exposure) const;
    void addEntity(const BoundingBox& box, VertexArray&& model, float health, const EntityBehavior& behavior);
    void removeEntity(size_t index);
    size_t count() const;

    // Entity Component System
    std::vector<PhysicsObject> m_PhysicsObjects;
    std::vector<VertexArray> m_Models;
    std::vector<EntityBehavior> m_BehaviorComponents; // TODO: implement behavior
    std::vector<float> m_Healths;
};

VertexArray createEntityWireframe(const glm::vec3& size);