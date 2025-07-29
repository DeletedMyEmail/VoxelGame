#include <gtest/gtest.h>
#include "Chunk.h"
#include <cstmlib/Log.h>

#include "Window.h"
#include "cstmlib/Profiling.h"

class TestClass : public testing::Test
{
protected:
    void SetUp() override {}
};

TEST_F(TestClass, Test1)
{
    EXPECT_TRUE(true);
}

void profileChunks()
{
    Window win;
    ChunkManager chunkManager;
    chunkManager.loadChunks({100, 100});
    Chunk c;
    auto res = REP_TEST([&]()
    {
        c = Chunk({100, 100});
    }, Chunk::BLOCKS_PER_CHUNK, 200, 200);

    LOG_INFO("Chunk Gen:\n{}\n", std::string(res));

    Chunk* leftChunk = chunkManager.getChunk({99, 100});
    Chunk* rightChunk = chunkManager.getChunk({101, 100});
    Chunk* frontChunk = chunkManager.getChunk({100, 101});
    Chunk* backChunk = chunkManager.getChunk({100, 99});

    res = REP_TEST([&]()
    {
        c.generateMeshData(leftChunk, rightChunk, frontChunk, backChunk);
    }, Chunk::BLOCKS_PER_CHUNK, 200, 200);

    LOG_INFO("Chunk Mesh:\n{}\n", std::string(res));
}

int main(int argc, char **argv)
{
    LOG_INIT();
    PROFILER_INIT();
    profileChunks();
    /*testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();*/
}