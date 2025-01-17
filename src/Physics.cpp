#include "Physics.h"

void PhysicsBody::update()
{
    m_Position += m_Velocity;
    m_Velocity = glm::mix(m_Velocity, glm::vec3(0), DAMPING);
}

float PhysicsBody::SweptAABB(const PhysicsBody& other, glm::vec3& normal) const
{
    float xInvEntry, yInvEntry, zInvEntry;
    float xInvExit, yInvExit, zInvExit;

    glm::vec3 otherPos = other.getPosition();
    glm::vec3 otherSize = other.getSize();

    // find the distance between the objects on the near and far sides for both x and y
    if (m_Velocity.x > 0.0f)
    {
        xInvEntry = otherPos.x - (m_Position.x + m_Size.x);
        xInvExit = (otherPos.x + otherSize.x) - m_Position.x;
    }
    else
    {
        xInvEntry = (otherPos.x + otherSize.x) - m_Position.x;
        xInvExit = otherPos.x - (m_Position.x + m_Size.x);
    }

    if (m_Velocity.y > 0.0f)
    {
        yInvEntry = otherPos.y - (m_Position.y + m_Size.y);
        yInvExit = (otherPos.y + otherSize.y) - m_Position.y;
    }
    else
    {
        yInvEntry = (otherPos.y + otherSize.y) - m_Position.y;
        yInvExit = otherPos.y - (m_Position.y + m_Size.y);
    }

    if (m_Velocity.z > 0.0f)
    {
        zInvEntry = otherPos.z - (m_Position.z + m_Size.z);
        zInvExit = (otherPos.z + otherSize.z) - m_Position.z;
    }
    else
    {
        zInvEntry = (otherPos.z + otherSize.z) - m_Position.z;
        zInvExit = otherPos.z - (m_Position.z + m_Size.z);
    }

    // find time of collision and time of leaving for each axis (if statement is to prevent divide by zero)
    float xEntry, yEntry, zEntry;
    float xExit, yExit, zExit;

    if (m_Velocity.x == 0.0f)
    {
        xEntry = -std::numeric_limits<float>::infinity();
        xExit = std::numeric_limits<float>::infinity();
    }
    else
    {
        xEntry = xInvEntry / m_Velocity.x;
        xExit = xInvExit / m_Velocity.x;
    }

    if (m_Velocity.y == 0.0f)
    {
        yEntry = -std::numeric_limits<float>::infinity();
        yExit = std::numeric_limits<float>::infinity();
    }
    else
    {
        yEntry = yInvEntry / m_Velocity.y;
        yExit = yInvExit / m_Velocity.y;
    }

    if (m_Velocity.z == 0.0f)
    {
        zEntry = -std::numeric_limits<float>::infinity();
        zExit = std::numeric_limits<float>::infinity();
    }
    else
    {
        zEntry = zInvEntry / m_Velocity.z;
        zExit = zInvExit / m_Velocity.z;
    }

    // find the earliest/latest times of collision
    float entryTime = std::max(std::max(xEntry, yEntry), zEntry);
    float exitTime = std::min(std::min(xExit, yExit), zExit);


    // if there was no collision
    if (entryTime > exitTime || xEntry < 0.0f && yEntry < 0.0f && zEntry < 0.0f || xEntry > 1.0f || yEntry > 1.0f || zEntry > 1.0f)
    {
        normal = glm::vec3(0);
        return 1.0f;
    }
    // if there was a collision
    // calculate normal of collided surface
    if (xEntry > yEntry && xEntry > zEntry)
    {
        if (xInvEntry < 0.0f)
        {
            normal.x = 1.0f;
            normal.y = 0.0f;
            normal.z = 0.0f;
        }
        else
        {
            normal.x = -1.0f;
            normal.y = 0.0f;
            normal.z = 0.0f;
        }
    }
    else if (yEntry > xEntry && yEntry > zEntry)
    {
        if (yInvEntry < 0.0f)
        {
            normal.x = 0.0f;
            normal.y = 1.0f;
            normal.z = 0.0f;
        }
        else
        {
            normal.x = 0.0f;
            normal.y = -1.0f;
            normal.z = 0.0f;
        }
    }
    else
    {
        if (zInvEntry < 0.0f)
        {
            normal.x = 0.0f;
            normal.y = 0.0f;
            normal.z = 1.0f;
        }
        else
        {
            normal.x = 0.0f;
            normal.y = 0.0f;
            normal.z = -1.0f;
        }
    }

    return entryTime;
}

bool PhysicsBody::solveCollision(const PhysicsBody& other)
{
    glm::vec3 normal(0);
    const float collisionTime = SweptAABB(other, normal);
    if (collisionTime == 1.0f)
        return false;

    m_Velocity *= collisionTime;

    const float remainingTime = 1.0f - collisionTime;

    // push
    const float magnitude = glm::length(m_Velocity) * remainingTime;
    float dot = glm::dot(m_Velocity, normal);

    if (dot > 0.0f)
        dot = -1.0f;
    else if (dot < 0.0f)
        dot = 1.0f;

    m_Velocity += normal * dot * magnitude;

    return true;
}
