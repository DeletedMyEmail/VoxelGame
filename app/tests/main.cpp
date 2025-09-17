#include <gtest/gtest.h>
#include "Chunk.h"
#include <cstmlib/Log.h>
#include "../include/Physics.h"
#include "Window.h"
#include "cstmlib/Profiling.h"
#include "../include/Physics.h"

class TestClass : public testing::Test
{
protected:
    void SetUp() override {}
};

void profile()
{

}

int main(int argc, char **argv)
{
    LOG_INIT();
    PROFILER_INIT();
    profile();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}