#include <gtest/gtest.h>

class TestClass : public testing::Test
{
protected:
    void SetUp() override {}
};

TEST_F(TestClass, Test1)
{
    EXPECT_EQ(1, 1);
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}