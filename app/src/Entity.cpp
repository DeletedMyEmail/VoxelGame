#include "../include/Entity.h"
#include "Chunk.h"
#include "glm/common.hpp"

void EntityManager::loadEntities()
{
}

void EntityManager::unloadEntities()
{
}

void EntityManager::updateEntities(const double dt, ChunkManager& chunkManager)
{
    const size_t n = count();
    for (size_t i = 0; i < n; ++i)
    {
        PhysicsObject& obj = m_PhysicsObjects.at(i);
        EntityBehavior& behavior = m_BehaviorComponents.at(i);
        applyGravity(obj, (float) dt);
        //behavior.movementFunction(obj);
        applyVelocityAndHandleCollisions(chunkManager, obj);
    }
}

void EntityManager::drawEntities(Renderer& renderer, const glm::mat4& viewProjection, const float exposure) const
{
    const size_t n = count();
    for (size_t i = 0; i < n; ++i)
        renderer.drawEntity(m_Models.at(i), m_PhysicsObjects.at(i).box.pos, viewProjection, exposure);
}

void EntityManager::addEntity(const BoundingBox& box, VertexArray&& model, float health, const EntityBehavior& behavior)
{
    m_PhysicsObjects.emplace_back(box, glm::vec3(0));
    m_Models.emplace_back(std::move(model));
    m_Healths.emplace_back(health);
    m_BehaviorComponents.emplace_back(behavior);
}

void EntityManager::removeEntity(const size_t index)
{
    m_PhysicsObjects.erase(m_PhysicsObjects.begin() + index);
    m_Models.erase(m_Models.begin() + index);
    m_Healths.erase(m_Healths.begin() + index);
    m_BehaviorComponents.erase(m_BehaviorComponents.begin() + index);
}

size_t EntityManager::count() const
{
    const size_t count = m_PhysicsObjects.size();
    assert(count == m_Models.size());
    assert(count == m_Healths.size());
    assert(count == m_BehaviorComponents.size());
    return count;
}

void applyGravity(PhysicsObject& obj, const float dtime)
{
    obj.velocity.y += GRAVITY * dtime;
    obj.velocity.y = glm::max(obj.velocity.y, -TERMINAL_VELOCITY);
}

static BoundingBox getBroadphaseBox(const PhysicsObject& obj);
static CollisionData getCollision(PhysicsObject& a, BoundingBox& b);
static void resolveCollision(PhysicsObject& a, const CollisionData& collisionData);

bool applyVelocityAndHandleCollisions(ChunkManager& chunkManager, PhysicsObject& obj)
{
    bool grounded = false;
    const int MAX_ITERATIONS = 4;
    for (int i = 0; i < MAX_ITERATIONS && glm::length(obj.velocity) > 0.001f; ++i)
    {
        auto [pos, size] = getBroadphaseBox(obj);

        CollisionData nearestCollision{glm::vec3(0), std::numeric_limits<float>::max()};

        for (int32_t x = glm::floor(pos.x); x < glm::ceil(pos.x + size.x); ++x)
        {
            for (int32_t y = glm::floor(pos.y); y < glm::ceil(pos.y + size.y); ++y)
            {
                for (int32_t z = glm::floor(pos.z); z < glm::ceil(pos.z + size.z); ++z)
                {
                    glm::ivec3 worldPos{x, y, z};
                    Chunk* chunk = chunkManager.getChunk(worldPosToChunkPos(worldPos));
                    if (!chunk)
                        continue;
                    glm::ivec3 blockPos = worldPosToChunkBlockPos(worldPos);
                    BLOCK_TYPE block = chunk->getBlockSafe(blockPos);
                    if (block == BLOCK_TYPE::INVALID || !isSolid(block))
                        continue;

                    BoundingBox blockBox{worldPos, glm::vec3(1.0f)};
                    CollisionData c = getCollision(obj, blockBox);

                    if (c.entryTime < nearestCollision.entryTime)
                        nearestCollision = c;
                }
            }
        }

        if (nearestCollision.entryTime > 1.0f)
        {
            obj.box.pos += obj.velocity;
            break;
        }

        if (nearestCollision.normal.y > 0.5f)
            grounded = true;

        resolveCollision(obj, nearestCollision);

        obj.velocity *= DAMPING;
    }

    return grounded;
}

static BoundingBox getBroadphaseBox(const PhysicsObject& obj)
{
    BoundingBox broadphaseBox{};
    broadphaseBox.pos.x = obj.velocity.x >= 0 ? obj.box.pos.x : obj.box.pos.x + obj.velocity.x;
    broadphaseBox.pos.y = obj.velocity.y >= 0 ? obj.box.pos.y : obj.box.pos.y + obj.velocity.y;
    broadphaseBox.pos.z = obj.velocity.z >= 0 ? obj.box.pos.z : obj.box.pos.z + obj.velocity.z;

    broadphaseBox.size.x = glm::abs(obj.velocity.x) + obj.box.size.x;
    broadphaseBox.size.y = glm::abs(obj.velocity.y) + obj.box.size.y;
    broadphaseBox.size.z = glm::abs(obj.velocity.z) + obj.box.size.z;

    return broadphaseBox;
}

static bool isColliding(const BoundingBox& a, const BoundingBox& b)
{
    return (a.pos.x < b.pos.x + b.size.x && b.pos.x < a.pos.x + a.size.x) &&
           (a.pos.y < b.pos.y + b.size.y && b.pos.y < a.pos.y + a.size.y) &&
           (a.pos.z < b.pos.z + b.size.z && b.pos.z < a.pos.z + a.size.z);
}

// swept AABB sources:
// https://stackoverflow.com/questions/35297190/swept-aabb-collision-detection-and-response-problems
// https://www.gamedev.net/tutorials/programming/general-and-gameplay-programming/swept-aabb-collision-detection-and-response-r3084/

static CollisionData getCollision(PhysicsObject& a, BoundingBox& b)
{
    assert((a.velocity.x != 0 || a.velocity.y != 0 || a.velocity.z != 0) && "Velocity should not be zero for this collision check to work");

    glm::vec3 distToEntry, distToExit;

    if (a.velocity.x >= 0)
    {
        distToEntry.x = b.pos.x - (a.box.pos.x + a.box.size.x);
        distToExit.x = (b.pos.x + b.size.x) - a.box.pos.x;
    }
    else
    {
        distToEntry.x = (b.pos.x + b.size.x) - a.box.pos.x;
        distToExit.x = b.pos.x - (a.box.pos.x + a.box.size.x);
    }

    if (a.velocity.y >= 0)
    {
        distToEntry.y = b.pos.y - (a.box.pos.y + a.box.size.y);
        distToExit.y = (b.pos.y + b.size.y) - a.box.pos.y;
    }
    else
    {
        distToEntry.y = (b.pos.y + b.size.y) - a.box.pos.y;
        distToExit.y = b.pos.y - (a.box.pos.y + a.box.size.y);
    }

    if (a.velocity.z >= 0)
    {
        distToEntry.z = b.pos.z - (a.box.pos.z + a.box.size.z);
        distToExit.z = (b.pos.z + b.size.z) - a.box.pos.z;
    }
    else
    {
        distToEntry.z = (b.pos.z + b.size.z) - a.box.pos.z;
        distToExit.z = b.pos.z - (a.box.pos.z + a.box.size.z);
    }

    glm::vec3 entryTime;
    glm::vec3 exitTime;

    if (a.velocity.x == 0.0f)
    {
        entryTime.x = (distToEntry.x < 0.0f) ? -std::numeric_limits<float>::infinity() : std::numeric_limits<float>::infinity();
        exitTime.x = (distToExit.x > 0.0f) ? std::numeric_limits<float>::infinity() : -std::numeric_limits<float>::infinity();
    }
    else
    {
        entryTime.x = distToEntry.x / a.velocity.x;
        exitTime.x = distToExit.x / a.velocity.x;
    }

    if (a.velocity.y == 0.0f)
    {
        entryTime.y = (distToEntry.y < 0.0f) ? -std::numeric_limits<float>::infinity() : std::numeric_limits<float>::infinity();
        exitTime.y = (distToExit.y > 0.0f) ? std::numeric_limits<float>::infinity() : -std::numeric_limits<float>::infinity();
    }
    else
    {
        entryTime.y = distToEntry.y / a.velocity.y;
        exitTime.y = distToExit.y / a.velocity.y;
    }

    if (a.velocity.z == 0.0f)
    {
        entryTime.z = (distToEntry.z < 0.0f) ? -std::numeric_limits<float>::infinity() : std::numeric_limits<float>::infinity();
        exitTime.z = (distToExit.z > 0.0f) ? std::numeric_limits<float>::infinity() : -std::numeric_limits<float>::infinity();
    }
    else
    {
        entryTime.z = distToEntry.z / a.velocity.z;
        exitTime.z = distToExit.z / a.velocity.z;
    }

    float maxEntryTime = glm::max(entryTime.x, glm::max(entryTime.y, entryTime.z));
    float minExitTime = glm::min(exitTime.x, glm::min(exitTime.y, exitTime.z));

    if (maxEntryTime > minExitTime || maxEntryTime < 0.0f || maxEntryTime > 1.0f)
        return {glm::vec3(0), std::numeric_limits<float>::max()};

    CollisionData collisionData{};
    collisionData.entryTime = maxEntryTime;

    if (maxEntryTime == entryTime.x)
        collisionData.normal = glm::vec3((a.velocity.x < 0 ? 1.0f : -1.0f), 0, 0);
    else if (maxEntryTime == entryTime.y)
        collisionData.normal = glm::vec3(0, (a.velocity.y < 0 ? 1.0f : -1.0f), 0);
    else
        collisionData.normal = glm::vec3(0, 0, (a.velocity.z < 0 ? 1.0f : -1.0f));

    return collisionData;
}

static void resolveCollision(PhysicsObject& a, const CollisionData& collisionData)
{
    a.box.pos += a.velocity * collisionData.entryTime;
    const float remainingTime = 1.0f - collisionData.entryTime;
    if (remainingTime > 0.0f)
    {
        const float dot = glm::dot(a.velocity, collisionData.normal);
        a.velocity -= collisionData.normal * dot;
    }
}

VertexArray createEntityWireframe(const glm::vec3& size)
{
    const std::array vertices =
    {
        0.0f, 0.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,
        size.x, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,

        size.x, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,
        size.x, 0.0f, size.z,  1.0f, 1.0f, 1.0f, 1.0f,

        size.x, 0.0f, size.z,  1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, size.z,    1.0f, 1.0f, 1.0f, 1.0f,

        0.0f, 0.0f, size.z,    1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,

        0.0f, size.y, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,
        size.x, size.y, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,

        size.x, size.y, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,
        size.x, size.y, size.z,1.0f, 1.0f, 1.0f, 1.0f,

        size.x, size.y, size.z,1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, size.y, size.z,  1.0f, 1.0f, 1.0f, 1.0f,

        0.0f, size.y, size.z,  1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, size.y, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,

        0.0f, 0.0f, 0.0f,      1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, size.y, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,

        size.x, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f, 1.0f,
        size.x, size.y, 0.0f,  1.0f, 1.0f, 1.0f, 1.0f,

        size.x, 0.0f, size.z,  1.0f, 1.0f, 1.0f, 1.0f,
        size.x, size.y, size.z,1.0f, 1.0f, 1.0f, 1.0f,

        0.0f, 0.0f, size.z,    1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, size.y, size.z,  1.0f, 1.0f, 1.0f, 1.0f
    };

    const GLuint vbo = createBuffer(vertices.data(), sizeof(float) * vertices.size());
    VertexBufferLayout layout;
    layout.pushFloat(3);
    layout.pushFloat(4);
    VertexArray vao;
    vao.addBuffer(vbo, layout);
    vao.vertexCount = vertices.size();
    return vao;
}

