#pragma once

#include "Physics.h"
#include "glm/vec3.hpp"

class Player
{
public:
    Player(const glm::vec3 pos) : m_PhysicsBody(pos, {1,1,1}, false) {}

    void takeDamage(const float damage) { m_Health -= damage; }
    void heal(const float health) { m_Health += health; }
    void increaseSpeed(const float speed) { m_Speed += speed; }

    PhysicsBody& getPhysics() { return m_PhysicsBody; }
    float getHealth() const { return m_Health; }
    float getSpeed() const { return m_Speed; }
private:
    PhysicsBody m_PhysicsBody;
    float m_Health = 100.0f;
    float m_Speed = 2.0f;
};
