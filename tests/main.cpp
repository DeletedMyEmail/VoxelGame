#include <gtest/gtest.h>
#include "Physics.h"
#include <glm/glm.hpp>

class PhysicsBodyTest : public testing::Test
{
protected:
    void SetUp() override {}
};

TEST_F(PhysicsBodyTest, PositiveXVelocityCollision)
{
    PhysicsBody body1({1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, false);
    body1.addVelocity({1.0f, 0.0f, 0.0f});

    PhysicsBody body2{glm::vec3(2.5f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), true};

    EXPECT_EQ(body1.solveCollision(body2), true);
}

TEST_F(PhysicsBodyTest, NegativeXVelocityCollision)
{
    PhysicsBody body1({2.5f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, false);
    body1.addVelocity({-1.0f, 0.0f, 0.0f});

    PhysicsBody body2{glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), true};

    EXPECT_EQ(body1.solveCollision(body2), true);
}

TEST_F(PhysicsBodyTest, NoCollision)
{
    PhysicsBody body1({0.0f, 4.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, false);
    body1.addVelocity({0.0f, 0.0f, 1.0f});

    PhysicsBody body2{glm::vec3(0.0f, 1.0f, 1.5f), glm::vec3(1.0f, 1.0f, 1.0f), true};

    EXPECT_EQ(body1.solveCollision(body2), false);
}

TEST_F(PhysicsBodyTest, PositiveYVelocityCollision)
{
    PhysicsBody body1({0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, false);
    body1.addVelocity({0.0f, 1.0f, 0.0f});

    PhysicsBody body2{{0.0f, 2.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, true};

    EXPECT_EQ(body1.solveCollision(body2), true);
}

TEST_F(PhysicsBodyTest, NegativeYVelocityCollision)
{
    PhysicsBody body1({0.0f, 2.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, false);
    body1.addVelocity({0.0f, -1.0f, 0.0f});

    PhysicsBody body2{{0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, true};

    EXPECT_EQ(body1.solveCollision(body2), true);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}