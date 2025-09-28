#include <gtest/gtest.h>
#include <cstmlib/Profiling.h>
#include <cstmlib/Log.h>

#include "Application.h"
#include "Chunk.h"

class TestClass : public testing::Test
{
protected:
    void SetUp() override {}
};

void profileChunkGen()
{
    const WorldGenerationData worldGenData(0);
    const glm::ivec3 pos(0);
    Chunk chunk;

    const auto res = REP_TEST([&]() { chunk = Chunk(pos, worldGenData); }, Chunk::BLOCKS_PER_CHUNK, 100, 100);
    LOG_INFO("Chunk Gen ---------\n{}", std::string(res));
}

void profileBaking()
{
    WorldGenerationData worldGenData(0);
    const glm::ivec3 pos(0);
    Chunk chunk(pos, worldGenData);
    ChunkManager chunkManager(gameConfig);

    // Chunk Mesh Baking (without neighbours) ---------
    const std::array<Chunk*, 6> neighbours0 = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    auto res = REP_TEST([&]() { chunk.generateMeshData(neighbours0); }, Chunk::BLOCKS_PER_CHUNK, 100, 100);
    LOG_INFO("Chunk Mesh Baking (without neighbours) ---------\n{}", std::string(res));

    // Chunk Mesh Baking (with neighbours pre-fetched) ---------
    const std::array<Chunk*, 6> neighbours1{
        // BACK, FRONT, LEFT, RIGHT, BOTTOM, TOP
        chunkManager.getChunk(pos + glm::ivec3{0, 0, -1}),
        chunkManager.getChunk(pos + glm::ivec3{0, 0, 1}),
        chunkManager.getChunk(pos + glm::ivec3{-1, 0, 0}),
        chunkManager.getChunk(pos + glm::ivec3{1, 0, 0}),
        chunkManager.getChunk(pos + glm::ivec3{0, -1, 0}),
        chunkManager.getChunk(pos + glm::ivec3{0, 1, 0})
    };
    res = REP_TEST([&]() { chunk.generateMeshData(neighbours1); }, Chunk::BLOCKS_PER_CHUNK, 100, 100);
    LOG_INFO("Chunk Mesh Baking (with neighbours pre-fetched) ---------\n{}", std::string(res));

    // Chunk Mesh Baking (with neighbours fetched live and populated map) ---------
    for (int32_t x = -int32_t(gameConfig.loadDistance); x <= gameConfig.loadDistance; x++)
        for (int32_t y = -int32_t(gameConfig.loadDistance); y <= gameConfig.loadDistance; y++)
            for (int32_t z = -int32_t(gameConfig.loadDistance); z <= gameConfig.loadDistance; z++)
                chunkManager.chunks.emplace(glm::ivec3{x, y, z}, Chunk(glm::ivec3{x, y, z}, worldGenData));

    res = REP_TEST(([&]()
    {
        const std::array<Chunk*, 6> neighbours2{
                // BACK, FRONT, LEFT, RIGHT, BOTTOM, TOP
                chunkManager.getChunk(pos + glm::ivec3{0, 0, -1}),
                chunkManager.getChunk(pos + glm::ivec3{0, 0, 1}),
                chunkManager.getChunk(pos + glm::ivec3{-1, 0, 0}),
                chunkManager.getChunk(pos + glm::ivec3{1, 0, 0}),
                chunkManager.getChunk(pos + glm::ivec3{0, -1, 0}),
                chunkManager.getChunk(pos + glm::ivec3{0, 1, 0})
            };
        chunk.generateMeshData(neighbours2);
    }), Chunk::BLOCKS_PER_CHUNK, 100, 100);
    LOG_INFO("Chunk Mesh Baking (with neighbours fetched live and populated map) ---------\n{}", std::string(res));

    {
        PROFILE_SCOPE();
        chunk.bakeMesh(); // * 32 on main thread => bottleneck
    }
}

int main(int argc, char **argv)
{
    LOG_INIT();
    PROFILER_INIT();
    WindowSettings settings;
    core::Application app(settings);
    profileChunkGen();
    profileBaking();
    PROFILER_END();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}