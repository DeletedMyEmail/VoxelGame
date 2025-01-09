#pragma once
#include "Chunk.h"

constexpr float GRAVITY = -0.0f;
constexpr float DAMPING =  0.2f;

class PhysicsBody {
public:
    PhysicsBody(const glm::vec3& pos, const glm::vec3& size, const bool isStatic) : m_Position(pos), m_Size(size), m_Velocity(0), m_Static(isStatic) {}

    void update();
    void move(const glm::vec3 offset) { m_Position += offset; }
    bool solveCollision(const PhysicsBody& other);
    void addVelocity(const glm::vec3& velocity) { m_Velocity += velocity; }

    bool isStatic() const { return m_Static; }
    glm::vec3 getVelocity() const { return m_Velocity; }
    glm::vec3 getPosition() const { return m_Position; }
    glm::vec3 getSize() const { return m_Size; }
private:
    float SweptAABB(const PhysicsBody& other, glm::vec3& normal) const;
private:
    glm::vec3 m_Position;
    glm::vec3 m_Size;
    glm::vec3 m_Velocity;
    bool m_Static;
};