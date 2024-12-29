#pragma once

#include <vector>
#include "Blockdata.h"

class Chunk {

public:
    Chunk(glm::uvec2 chunkPosition);
    ~Chunk();

    const std::vector<Blockdata>& getBlocks() { return m_Blocks; }
    const glm::uvec2& getPosition() const { return m_ChunkPosition; }
private:
    static constexpr unsigned int CHUNK_SIZE = 16;
    static constexpr unsigned int MAX_HEIGHT = 4;

    glm::uvec2 m_ChunkPosition;
    std::vector<Blockdata> m_Blocks;
};
