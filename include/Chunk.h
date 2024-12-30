#pragma once

#include <vector>
#include "Blockdata.h"

class Chunk {

public:
    Chunk(glm::uvec2 chunkPosition);
    ~Chunk();

    const std::vector<Blockdata>& getBlocks() { return m_Blocks; }
    const glm::uvec2& getPosition() const { return m_ChunkPosition; }
public:
    static constexpr unsigned char CHUNK_SIZE = 16;
    static constexpr unsigned char MAX_HEIGHT = 16;
private:

    glm::uvec2 m_ChunkPosition;
    std::vector<Blockdata> m_Blocks;
};
