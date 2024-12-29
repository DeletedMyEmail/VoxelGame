#include "Chunk.h"

Chunk::Chunk()
    : m_Heights(), m_Blocks()
{
    for (int x = 0; x < CHUNK_SIZE; x++)
    {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
            m_Heights[x][z] = x * z;
        }
    }

    for (unsigned int x = 0; x < CHUNK_SIZE; x++)
    {
        for (unsigned int y = 0; y < MAX_HEIGHT; y++)
        {
            for (unsigned int z = 0; z < CHUNK_SIZE; z++)
            {
                BlockType type = SOLID;
                if (y > m_Heights[x][z])
                    type = AIR;

                m_Blocks[x][y][z] = {type, {x, y, z}};
            }
        }
    }
}

Chunk::~Chunk() = default;

