#include <gtest/gtest.h>
#include "Physics.h"
#include <glm/glm.hpp>

class PhysicsBodyTest : public ::testing::Test
{
protected:
    PhysicsBody body1{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), false};
    PhysicsBody body2{glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), false};

    void SetUp() override {}
};

TEST_F(PhysicsBodyTest, PositiveVelocityCollision) {
    body1.setVelocity(glm::vec3(1.0f, 0.0f, 0.0f));
    body2.setPosition(glm::vec3(1.8f, 0.0f, 0.0f));

    glm::vec3 normal;
    float collisionTime = body1.SweptAABB(body2, normal);

    EXPECT_LT(collisionTime, 1.0f);
    EXPECT_EQ(normal, glm::vec3(-1.0f, 0.0f, 0.0f));
}

TEST_F(PhysicsBodyTest, NegativeVelocityCollision) {
    body1.setVelocity(glm::vec3(-1.0f, 0.0f, 0.0f));
    body2.setPosition(glm::vec3(-1.8f, 0.0f, 0.0f));

    glm::vec3 normal;
    float collisionTime = body1.SweptAABB(body2, normal);

    EXPECT_LT(collisionTime, 1.0f);
    EXPECT_EQ(normal, glm::vec3(1.0f, 0.0f, 0.0f));
}

TEST_F(PhysicsBodyTest, NoCollision) {
    body1.setVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
    body2.setPosition(glm::vec3(5.0f, 0.0f, 0.0f));

    glm::vec3 normal;
    float collisionTime = body1.SweptAABB(body2, normal);

    EXPECT_EQ(collisionTime, 1.0f);
    EXPECT_EQ(normal, glm::vec3(0.0f, 0.0f, 0.0f));
}

TEST_F(PhysicsBodyTest, EdgeCollision) {
    body1.setVelocity(glm::vec3(1.0f, 0.0f, 0.0f));
    body2.setPosition(glm::vec3(1.0f, 0.0f, 0.0f));

    glm::vec3 normal;
    float collisionTime = body1.SweptAABB(body2, normal);

    EXPECT_EQ(collisionTime, 0.0f);
    EXPECT_EQ(normal, glm::vec3(-1.0f, 0.0f, 0.0f));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}