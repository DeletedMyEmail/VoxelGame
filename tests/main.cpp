#include <gtest/gtest.h>
#include "Chunk.h"
#include <cstmlib/Log.h>

#include "Window.h"
#include "WorldGeneration.h"
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
    chunkManager.loadChunks({100, 2, 100});
    Chunk c;
    auto res = REP_TEST([&]()
    {
        c = Chunk({100, 2, 100});
    }, Chunk::BLOCKS_PER_CHUNK, 200, 200);

    LOG_INFO("Chunk Gen:\n{}\n", std::string(res));

    Chunk* leftChunk = chunkManager.getLoadedChunk({99, 2, 100});
    Chunk* rightChunk = chunkManager.getLoadedChunk({101, 2, 100});
    Chunk* frontChunk = chunkManager.getLoadedChunk({100, 2, 101});
    Chunk* backChunk = chunkManager.getLoadedChunk({100, 2, 99});
    Chunk* topChunk = chunkManager.getLoadedChunk({100, 3, 100});
    Chunk* bottomChunk = chunkManager.getLoadedChunk({100, 1, 100});

    res = REP_TEST([&]()
    {
        c.generateMeshData(leftChunk, rightChunk, frontChunk, backChunk, topChunk, bottomChunk);
    }, Chunk::BLOCKS_PER_CHUNK, 200, 200);

    LOG_INFO("Chunk Mesh:\n{}\n", std::string(res));
}

void profileNoise()
{
    uint32_t height = getHeightAt({0,0});
    uint32_t i = 0;
    auto res = REP_TEST([&]()
    {
        height = getHeightAt({i++, i + 100});
    }, 1, 200, 200);

    LOG_INFO("Noise:\n{}\n", std::string(res));
}

int main(int argc, char **argv)
{
    LOG_INIT();
    PROFILER_INIT();
    profileChunks();
    profileNoise();
    /*testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();*/
}