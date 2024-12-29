#pragma once

#include "Blockdata.h"

class Chunk {

public:
    Chunk();
    ~Chunk();

public:
    static const unsigned int CHUNK_SIZE = 16;
    static const unsigned int MAX_HEIGHT = 255;

    unsigned int m_Heights[CHUNK_SIZE][CHUNK_SIZE];
    Blockdata m_Blocks[CHUNK_SIZE][MAX_HEIGHT][CHUNK_SIZE];
};
