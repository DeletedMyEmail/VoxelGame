#include "Chunk.h"
#include "OpenGLHelper.h"


static uint32_t getBlockIndex(const glm::uvec3& pos) { return pos.x + pos.y * Chunk::CHUNK_SIZE + pos.z * Chunk::CHUNK_SIZE * Chunk::MAX_HEIGHT; }

uint32_t noiseToHeight(const float value)
{
    const float normalized = (value + 1.0f) * 0.5f;
    return Chunk::MIN_GEN_HEIGHT + normalized * (Chunk::MAX_GEN_HEIGHT - Chunk::MIN_GEN_HEIGHT);
}

Chunk::Chunk()
    : chunkPosition({0}), blocks{}
{
}

Chunk::Chunk(const glm::uvec2& chunkPosition, const FastNoiseLite& noise, const BIOME biome)
    : chunkPosition(chunkPosition), blocks{}
{
    BLOCK_TYPE defaultBlock = defaultBiomeBlock(biome);
    for (uint32_t x = 0; x < CHUNK_SIZE; x++)
    {
        for (uint32_t z = 0; z < CHUNK_SIZE; z++)
        {
            const glm::uvec3 worldPos = chunkPosToWorldBlockPos(chunkPosition) + glm::uvec3{x, 0, z};
            const uint32_t localHeight = noiseToHeight(noise.GetNoise((float) worldPos.x, (float) worldPos.z));

            for (uint32_t y = 0; y < MAX_HEIGHT; y++)
            {
                const auto index = getBlockIndex({x,y,z});

                if (y >= localHeight)
                    blocks[index] = BLOCK_TYPE::AIR;
                else
                    blocks[index] = y > localHeight-5 ? defaultBlock : BLOCK_TYPE::STONE;
            }
        }
    }
}

void Chunk::bake(Chunk* neighborChunks[3][3])
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

                for (uint32_t i = 0; i < 6; i++)
                {
                    if (i == BOTTOM && y == 0)
                        continue;
                    if (i == TOP && y == MAX_HEIGHT - 1)
                        continue;

                    glm::uvec3 neighbourBlockPos;
                    switch (i)
                    {
                        case BACK: neighbourBlockPos = {x, y, z - 1}; break;
                        case FRONT: neighbourBlockPos = {x, y, z + 1}; break;
                        case LEFT: neighbourBlockPos = {x - 1, y, z}; break;
                        case RIGHT: neighbourBlockPos = {x + 1, y, z}; break;
                        case TOP: neighbourBlockPos = {x, y + 1, z}; break;
                        case BOTTOM: neighbourBlockPos = {x, y - 1, z}; break;
                        default: assert(false);
                    }

                    if (inBounds(neighbourBlockPos) && getBlockUnsafe(neighbourBlockPos) != BLOCK_TYPE::AIR)
                        continue;
                    if (!inBounds(neighbourBlockPos))
                    {
                        // check if a neighboring chunk has a covering block
                        int dx = 1, dz = 1;
                        glm::uvec3 blockPosInOtherChunk = neighbourBlockPos;

                        if (neighbourBlockPos.x == CHUNK_SIZE) { dx = 2; blockPosInOtherChunk.x = 0; }
                        else if (neighbourBlockPos.x > CHUNK_SIZE) { dx = 0; blockPosInOtherChunk.x = CHUNK_SIZE - 1; }

                        if (neighbourBlockPos.z == CHUNK_SIZE) { dz = 2; blockPosInOtherChunk.z = 0; }
                        else if (neighbourBlockPos.z > CHUNK_SIZE) { dz = 0; blockPosInOtherChunk.z = CHUNK_SIZE - 1; }

                        assert(dx != 1 || dz != 1);

                        const Chunk* neighborChunk = neighborChunks[dx][dz];
                        assert(!neighborChunk || neighborChunk->getBlockUnsafe(blockPosInOtherChunk) != BLOCK_TYPE::INVALID);
                        if (neighborChunk && neighborChunk->getBlockUnsafe(blockPosInOtherChunk) != BLOCK_TYPE::AIR)
                            continue;
                    }

                    const glm::uvec2 atlasOffset = getAtlasOffset(block, i);
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

void Chunk::setBlockUnsafe(const glm::uvec3& pos, const BLOCK_TYPE block)
{
    blocks[getBlockIndex(pos)] = block;
    isDirty = true;
}

void Chunk::setBlockSafe(const glm::uvec3& pos, BLOCK_TYPE block)
{
    if (inBounds(pos))
    {
        setBlockUnsafe(pos, block);
        isDirty = true;
    }
    else
        LOG_WARN("BLock not found in chunk: ({}, {}, {})", pos.x, pos.y, pos.z);
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

BLOCK_TYPE defaultBiomeBlock(const BIOME b)
{
    switch (b)
    {
        case PLAINS:
            return BLOCK_TYPE::GRASS;
        case DESERT:
            return BLOCK_TYPE::SAND;
        case FOREST:
            return BLOCK_TYPE::WOOD;
        case MOUNTAIN:
            return BLOCK_TYPE::STONE;
        case HILLS:
            return BLOCK_TYPE::GRASS;
        default:
            return BLOCK_TYPE::INVALID;
    }

}

Chunk* getChunk(std::vector<Chunk>& chunks, const glm::uvec2& chunkPos)
{
    const auto it = std::ranges::find_if(chunks, [chunkPos](const Chunk& c) {
        return c.chunkPosition == chunkPos;
    });

    if (it != chunks.end())
        return &*it;

    return nullptr;
};