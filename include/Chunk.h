#pragma once

#include "VertexArray.h"

enum class BLOCK_TYPE : unsigned char {
    AIR,
    TEST,
    GRASS,
    STONE,
};

class Chunk
{
public:
    Chunk();
    Chunk(glm::uvec2 chunkPosition, unsigned char** heightMap);
    ~Chunk() = default;

    void generateMesh();
    static bool inBounds(const glm::uvec3 pos) { return pos.x < CHUNK_SIZE && pos.y < MAX_HEIGHT && pos.z < CHUNK_SIZE; };

    const VertexArray& getMesh() { return m_VAO; }
    BLOCK_TYPE getBlock(const glm::uvec3 pos) const { return m_Blocks[getBlockIndex(pos)]; }
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
    BLOCK_TYPE m_Blocks[BLOCKS_PER_CHUNK];
};

glm::uvec2 getAtlasOffset(BLOCK_TYPE block);
glm::uvec2 getChunkPos(const glm::vec3& pos);