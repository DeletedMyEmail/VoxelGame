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
    profileChunks();
    /*testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();*/
}