#include "Chunk.h"

#include <ctime>

#include "Log.h"
#include "Noise.h"

Chunk::Chunk()
    : m_ChunkPosition({0}), m_Blocks()
{
}

Chunk::Chunk(const glm::uvec2 chunkPosition, unsigned char** heightMap)
    : m_ChunkPosition(chunkPosition)
{
    m_Blocks.reserve(MAX_HEIGHT * CHUNK_SIZE * CHUNK_SIZE);

    for (unsigned char x = 0; x < CHUNK_SIZE; x++)
    {
        for (unsigned char z = 0; z < CHUNK_SIZE; z++)
        {
            const unsigned char localHeight = heightMap[x + chunkPosition.x * CHUNK_SIZE][z + chunkPosition.y * CHUNK_SIZE] % MAX_HEIGHT;
            for (unsigned char y = 0; y < localHeight; y++)
            {
                const auto atlasOffset = y > localHeight-3 ? GRASS_TEXTURE_OFFSET : STONE_TEXTURE_OFFSET;
                m_Blocks.emplace_back(Blockdata{SOLID, atlasOffset, {x, y, z}});
            }
        }
    }
}

Chunk::~Chunk() = default;

