#include <gtest/gtest.h>
#include "Chunk.h"
#include <cstmlib/Log.h>

#include "Physics.h"
#include "Window.h"
#include "GameWorld.h"
#include "cstmlib/Profiling.h"

class TestClass : public testing::Test
{
protected:
    void SetUp() override {}
};

TEST_F(TestClass, CollisionTestFullOverlap)
{
    /*PhysicsObject a;
    a.box.pos = {0, 0, 0};
    a.box.size = {1, 1, 1};
    a.velocity = {0, 0, 0};

    BoundingBox b;
    b.pos = {0, 0, 0};
    b.size = {1, 1, 1};

    CollisionData collisionData = getCollision(a, b);
    EXPECT_EQ(collisionData.entryTime, 0.0f);
    EXPECT_EQ(collisionData.normal, glm::vec3(0, 0, 0));*/
}

TEST_F(TestClass, CollisionTestNoHit)
{
    PhysicsObject a;
    a.box.pos = {0, 0, 0};
    a.box.size = {1, 1, 1};
    a.velocity = {0.5, 0.5, -1};

    BoundingBox b;
    b.pos = {2, 2, 2};
    b.size = {1, 1, 1};

    CollisionData collisionData = getCollision(a, b);
    EXPECT_EQ(collisionData.entryTime, std::numeric_limits<float>::max());
    EXPECT_EQ(collisionData.normal, glm::vec3(0, 0, 0));
}

TEST_F(TestClass, CollisionTestPerfectHitPositiveX)
{
    PhysicsObject a;
    a.box.pos = {0, 0, 0};
    a.box.size = {1, 1, 1};
    a.velocity = {1, 0, 0};

    BoundingBox b;
    b.pos = {2, 0, 0};
    b.size = {1, 1, 1};

    CollisionData collisionData = getCollision(a, b);
    EXPECT_EQ(collisionData.entryTime, 1.0f);
    EXPECT_EQ(collisionData.normal, glm::vec3(-1, 0, 0));
}

TEST_F(TestClass, CollisionTestPerfectHitNegativeX)
{
    PhysicsObject a;
    a.box.pos = {2, 0, 0};
    a.box.size = {1, 1, 1};
    a.velocity = {-1, 0, 0};

    BoundingBox b;
    b.pos = {0, 0, 0};
    b.size = {1, 1, 1};

    CollisionData collisionData = getCollision(a, b);
    EXPECT_EQ(collisionData.entryTime, 1.0f);
    EXPECT_EQ(collisionData.normal, glm::vec3(1, 0, 0));
}

TEST_F(TestClass, CollisionTestPerfectHitNegativeY)
{
    PhysicsObject a;
    a.box.pos = {0, 2, 0};
    a.box.size = {1, 1, 1};
    a.velocity = {0, -1, 0};

    BoundingBox b;
    b.pos = {0, 0, 0};
    b.size = {1, 1, 1};

    CollisionData collisionData = getCollision(a, b);
    EXPECT_EQ(collisionData.entryTime, 1.0f);
    EXPECT_EQ(collisionData.normal, glm::vec3(0, 1, 0));
}

TEST_F(TestClass, CollisionTestPerfectHitPositiveY)
{
    PhysicsObject a;
    a.box.pos = {0, 0, 0};
    a.box.size = {1, 1, 1};
    a.velocity = {0, 1, 0};

    BoundingBox b;
    b.pos = {0, 2, 0};
    b.size = {1, 1, 1};

    CollisionData collisionData = getCollision(a, b);
    EXPECT_EQ(collisionData.entryTime, 1.0f);
    EXPECT_EQ(collisionData.normal, glm::vec3(0, -1, 0));
}

TEST_F(TestClass, CollisionTestHitOnHalfway)
{
    PhysicsObject a;
    a.box.pos = {0, 0, 0};
    a.box.size = {1, 1, 1};
    a.velocity = {1, 1, 1};

    BoundingBox b;
    b.pos = {1.5, 1.5, 1.5};
    b.size = {1, 1, 1};

    CollisionData collisionData = getCollision(a, b);
    EXPECT_EQ(collisionData.entryTime, 0.5f);
    EXPECT_EQ(collisionData.normal, glm::vec3(-1, 0, 0));
}

void profileChunks()
{
    Window win;
    ChunkManager chunkManager;
    glm::ivec3 chunkPos{-1, 2, -2};
    chunkManager.loadChunks(chunkPos);
    Chunk c;
    auto res = REP_TEST([&]()
    {
        c = Chunk(chunkPos);
    }, Chunk::BLOCKS_PER_CHUNK, 200, 200);

    LOG_INFO("Chunk Gen:\n{}\n", std::string(res));

    Chunk* leftChunk = chunkManager.getChunk(chunkPos + glm::ivec3{-1, 0, 0});
    Chunk* rightChunk = chunkManager.getChunk(chunkPos + glm::ivec3{0, -1, 0});
    Chunk* frontChunk = chunkManager.getChunk(chunkPos + glm::ivec3{0, 0, 1});
    Chunk* backChunk = chunkManager.getChunk(chunkPos + glm::ivec3{0, 0, -1});
    Chunk* topChunk = chunkManager.getChunk(chunkPos + glm::ivec3{-1, 0, 0});
    Chunk* bottomChunk = chunkManager.getChunk(chunkPos + glm::ivec3{0, -1, 0});

    res = REP_TEST([&]()
    {
        c.generateMeshData(leftChunk, rightChunk, frontChunk, backChunk, topChunk, bottomChunk);
        c.bakeMesh();
    }, Chunk::BLOCKS_PER_CHUNK, 200, 200);

    LOG_INFO("Chunk Mesh:\n{}\n", std::string(res));
}

int main(int argc, char **argv)
{
    LOG_INIT();
    PROFILER_INIT();
    //profileChunks();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}