#pragma once

#include <glm/geometric.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

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
    const glm::vec3& getVelocity() const { return m_Velocity; }
    const glm::vec3& getPosition() const { return m_Position; }
    const glm::vec3& getSize() const { return m_Size; }
protected:
    float SweptAABB(const PhysicsBody& other, glm::vec3& normal) const;
    bool sweptBroadphase(const PhysicsBody& other) const;
private:
    glm::vec3 m_Position;
    glm::vec3 m_Size;
    glm::vec3 m_Velocity;
    bool m_Static;
};

struct Ray
{
    glm::vec3 origin, direction;
};