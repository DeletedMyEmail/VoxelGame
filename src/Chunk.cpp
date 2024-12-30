#include "Chunk.h"

#include <ctime>

#include "Noise.h"

Chunk::Chunk(const glm::uvec2 chunkPosition)
    : m_ChunkPosition(chunkPosition)
{
    m_Blocks.reserve(MAX_HEIGHT * CHUNK_SIZE * CHUNK_SIZE);

    unsigned char** heightMap = genPerlinMap(Chunk::CHUNK_SIZE, Chunk::CHUNK_SIZE, MAX_HEIGHT/2, MAX_HEIGHT , time(nullptr));

    for (unsigned char x = 0; x < CHUNK_SIZE; x++)
    {
        for (unsigned char z = 0; z < CHUNK_SIZE; z++)
        {
            const unsigned char localHeight = heightMap[x][z] % MAX_HEIGHT;
            for (unsigned char y = 0; y < localHeight; y++)
            {
                m_Blocks.emplace_back(Blockdata{SOLID, y > localHeight-3 ? GRASS_TEXTURE_OFFSET : STONE_TEXTURE_OFFSET, {x, y, z}});
            }
        }
    }

    freePerlinMap(heightMap, CHUNK_SIZE);
}

Chunk::~Chunk() = default;

