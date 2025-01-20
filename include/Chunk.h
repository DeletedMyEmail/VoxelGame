#pragma once

#include "Blockdata.h"
#include "VertexArray.h"

struct Mesh
{
    VertexArray* vao;
    unsigned int faceCount;
};

class Chunk
{
public:
    Chunk();
    Chunk(glm::uvec2 chunkPosition, unsigned char** heightMap);
    ~Chunk() = default;

    void generateMesh();
    Mesh getMesh() { return {&m_VAO, m_FaceCount}; }

    const Blockdata& getBlock(const glm::uvec3 pos) const { return m_Blocks[getBlockIndex(pos)]; }
    Blockdata* getBlocks() { return m_Blocks; }
    const glm::uvec2& getPosition() const { return m_ChunkPosition; }
private:
    void createBlocks(unsigned char** heightMap);
    void selectFaces(std::vector<GLuint>& buffer);
    unsigned int getUncoveredFaces(glm::uvec3 pos) const;
    static unsigned int getBlockIndex(const glm::uvec3 pos) { return pos.x + CHUNK_SIZE * (pos.y + MAX_HEIGHT * pos.z);}
public:
    static constexpr unsigned int CHUNK_SIZE = 16;
    static constexpr unsigned int MAX_HEIGHT = 16;
    static constexpr unsigned int BLOCKS_PER_CHUNK = CHUNK_SIZE * CHUNK_SIZE * MAX_HEIGHT;
private:
    VertexArray m_VAO;
    glm::uvec2 m_ChunkPosition;
    Blockdata m_Blocks[BLOCKS_PER_CHUNK];
    unsigned int m_FaceCount = 0;
};
