#include "Chunk.h"
#include "OpenGLHelper.h"

static uint8_t getBlockIndex(const glm::uvec3& pos) { return pos.x + pos.y * Chunk::CHUNK_SIZE + pos.z * Chunk::CHUNK_SIZE * Chunk::MAX_HEIGHT; }

static bool inBounds(const glm::uvec3& pos) { return pos.x < Chunk::CHUNK_SIZE && pos.y < Chunk::MAX_HEIGHT && pos.z < Chunk::CHUNK_SIZE; };

uint32_t noiseToHeight(const float value) { return Chunk::MIN_HEIGHT + ((value + 1.0f) / 2.0f) * (Chunk::MAX_HEIGHT - Chunk::MIN_HEIGHT); }

Chunk::Chunk()
    : chunkPosition({0}), blocks{}
{
}

Chunk::Chunk(const glm::uvec2& chunkPosition, const FastNoiseLite& noise)
    : chunkPosition(chunkPosition), blocks{}
{
    for (uint8_t x = 0; x < CHUNK_SIZE; x++)
    {
        for (uint8_t z = 0; z < CHUNK_SIZE; z++)
        {
            const uint8_t localHeight = noiseToHeight(noise.GetNoise((float) x, (float) z));

            for (uint8_t y = 0; y < MAX_HEIGHT; y++)
            {
                const auto index = getBlockIndex({x,y,z});

                if (y >= localHeight)
                    blocks[index] = BLOCK_TYPE::AIR;
                else
                {
                    const BLOCK_TYPE type = y > localHeight-3 ? BLOCK_TYPE::GRASS : BLOCK_TYPE::STONE;
                    blocks[index] = type;
                }

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
                    blockdata packedData = (i << 28) | (x << 24) | (y << 20) | (z << 12) | (atlasOffset.x << 8) | (atlasOffset.y << 4);
                    for (uint32_t j = 0; j < 6; j++)
                        buffer.push_back(packedData); // TODO: magic number

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