#include "Chunk.h"

Chunk::Chunk(const glm::uvec2 chunkPosition)
    : m_ChunkPosition(chunkPosition)
{
    m_Blocks.reserve(MAX_HEIGHT * CHUNK_SIZE * CHUNK_SIZE);

    for (unsigned int x = 0; x < CHUNK_SIZE; x++)
    {
        for (unsigned int y = 0; y < MAX_HEIGHT; y++)
        {
            for (unsigned int z = 0; z < CHUNK_SIZE; z++)
            {
                m_Blocks.emplace_back(Blockdata{SOLID, GRASS_TEXTURE_OFFSET, {x, y, z}});
            }
        }
    }
}

Chunk::~Chunk() = default;

