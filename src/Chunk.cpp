#include "Chunk.h"
#include "OpenGLHelper.h"


static uint32_t getBlockIndex(const glm::uvec3& pos) { return pos.x + pos.y * Chunk::CHUNK_SIZE + pos.z * Chunk::CHUNK_SIZE * Chunk::MAX_HEIGHT; }

static bool inBounds(const glm::uvec3& pos) { return pos.x < Chunk::CHUNK_SIZE && pos.y < Chunk::MAX_HEIGHT && pos.z < Chunk::CHUNK_SIZE; };

uint32_t noiseToHeight(const float value)
{
    const float normalized = (value + 1.0f) * 0.5f;
    return Chunk::MIN_GEN_HEIGHT + normalized * (Chunk::MAX_GEN_HEIGHT - Chunk::MIN_GEN_HEIGHT);
}

Chunk::Chunk()
    : chunkPosition({0}), blocks{}
{
}

Chunk::Chunk(const glm::uvec2& chunkPosition, const FastNoiseLite& noise)
    : chunkPosition(chunkPosition), blocks{}
{
    for (uint32_t x = 0; x < CHUNK_SIZE; x++)
    {
        for (uint32_t z = 0; z < CHUNK_SIZE; z++)
        {
            const glm::vec3 worldPos = chunkPosToWorldPos(chunkPosition) + glm::vec3{x, 0.0f, z};
            const uint32_t localHeight = noiseToHeight(noise.GetNoise(worldPos.x, worldPos.z));

            for (uint32_t y = 0; y < MAX_HEIGHT; y++)
            {
                const auto index = getBlockIndex({x,y,z});

                if (y >= localHeight)
                    blocks[index] = BLOCK_TYPE::AIR;
                else
                    blocks[index] = y == localHeight-1 ? BLOCK_TYPE::GRASS_FULL : y > localHeight-3 ? BLOCK_TYPE::GRASS : BLOCK_TYPE::STONE;

            }
        }
    }
}

// TODO: check nearby chunks
void Chunk::bake()
{
    std::vector<blockdata> buffer;
    uint32_t faceCount = 0;
    for (uint32_t x = 0; x < CHUNK_SIZE; x++)
    {
        for (uint32_t y = 0; y < MAX_HEIGHT; y++)
        {
            for (uint32_t z = 0; z < CHUNK_SIZE; z++)
            {
                const auto& block = getBlockUnsafe({x,y,z});

                assert(block != BLOCK_TYPE::INVALID);
                if (block == BLOCK_TYPE::AIR)
                    continue;

                // TODO: only for uncovered faces
                for (uint32_t i = 0; i < 6; i++)
                {
                    const glm::uvec2 atlasOffset = getAtlasOffset(block, i);
                    assert(x < 16);
                    assert(y < 256);
                    assert(z < 16);
                    assert(atlasOffset.x < 16);
                    assert(atlasOffset.y < 16);
                    blockdata packedData = (i << 28) | (x << 24) | (y << 16) | (z << 12) | (atlasOffset.x << 8) | (atlasOffset.y << 4);
                    for (uint32_t j = 0; j < 6; j++)
                        buffer.push_back(packedData);

                    faceCount++;
                }
            }
        }
    }

    vao.clear();
    const GLuint vbo = createBuffer(buffer.data(), buffer.size() * sizeof(blockdata));
    VertexBufferLayout layout;
    layout.pushUInt(1);
    vao.addBuffer(vbo, layout);
    vao.vertexCount = faceCount * 6;
    isDirty = false;
}

BLOCK_TYPE Chunk::getBlockUnsafe(const glm::uvec3& pos) const
{
    return blocks[getBlockIndex(pos)];
}

BLOCK_TYPE Chunk::getBlockSafe(const glm::uvec3& pos) const
{
    if (inBounds(pos))
        return getBlockUnsafe(pos);

    return BLOCK_TYPE::INVALID;
}


FastNoiseLite createBiomeNoise(const BIOME b, const int32_t seed)
{
    FastNoiseLite noise(seed);
    switch (b)
    {
    case PLAINS:
        noise.SetNoiseType(FastNoiseLite::NoiseType_ValueCubic);
        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFrequency(0.007f);
        break;
    case DESERT:
        noise.SetNoiseType(FastNoiseLite::NoiseType_Value);
        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFrequency(0.005f);
        break;
    case FOREST:
        noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
        noise.SetFractalType(FastNoiseLite::FractalType_None);
        noise.SetFrequency(0.002f);
        break;
    case MOUNTAIN:
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFrequency(0.008f);
        break;
    case HILLS:
        noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
        noise.SetFractalType(FastNoiseLite::FractalType_Ridged);
        noise.SetFrequency(0.007f);
        break;
    }

    return noise;
}