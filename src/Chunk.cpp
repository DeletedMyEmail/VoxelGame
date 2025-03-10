#include "Chunk.h"
#include "Renderer.h"

glm::uvec2 getAtlasOffset(const BLOCK_TYPE block)
{
    switch (block)
    {
        case BLOCK_TYPE::GRASS:
            return {1,0};
        case BLOCK_TYPE::STONE:
            return {1,1};
        default:
            return {0,0};
    }
}

glm::uvec2 getChunkPos(const glm::vec3& pos)
{
    return {static_cast<unsigned int>(pos.x) / Chunk::CHUNK_SIZE, static_cast<unsigned int>(pos.z) / Chunk::CHUNK_SIZE};
}

Chunk::Chunk()
    : m_ChunkPosition({0}), m_Blocks{}
{
}

Chunk::Chunk(const glm::uvec2 chunkPosition, unsigned char** heightMap)
    : m_ChunkPosition(chunkPosition), m_Blocks()
{
    createBlocks(heightMap);
    generateMesh();
}

void Chunk::generateMesh()
{
    m_VAO.clear();

    std::vector<GLuint> vertices;
    vertices.reserve(CHUNK_SIZE * CHUNK_SIZE * 6);

    selectFaces(vertices);

    const auto vBuffer = std::make_shared<VertexBuffer>(sizeof(GLuint) * vertices.size(), vertices.data());
    VertexBufferLayout layout;
    layout.push<GLuint>(1);

    m_VAO.addBuffer(vBuffer, layout);
}

void Chunk::createBlocks(unsigned char** heightMap)
{
    for (unsigned char x = 0; x < CHUNK_SIZE; x++)
    {
        for (unsigned char z = 0; z < CHUNK_SIZE; z++)
        {
            const unsigned char localHeight = heightMap[x + m_ChunkPosition.x * CHUNK_SIZE][z + m_ChunkPosition.y * CHUNK_SIZE] % MAX_HEIGHT;

            for (unsigned char y = 0; y < MAX_HEIGHT; y++)
            {
                const auto index = getBlockIndex({x,y,z});

                if (y >= localHeight)
                {
                    m_Blocks[index] = BLOCK_TYPE::AIR;
                }
                else
                {
                    const BLOCK_TYPE type = y > localHeight-3 ? BLOCK_TYPE::GRASS : BLOCK_TYPE::STONE;
                    m_Blocks[index] = type;
                }

            }
        }
    }
}

unsigned int Chunk::getUncoveredFaces(glm::uvec3 pos) const
{
    unsigned int uncoveredFaces = 0;

    if (pos.x == 0 || m_Blocks[getBlockIndex({pos.x-1, pos.y, pos.z})] == BLOCK_TYPE::AIR)
        uncoveredFaces |= 1 << LEFT_FACE_INDEX;
    if (pos.x == CHUNK_SIZE-1 || m_Blocks[getBlockIndex({pos.x+1, pos.y, pos.z})] == BLOCK_TYPE::AIR)
        uncoveredFaces |= 1 << RIGHT_FACE_INDEX;
    if (pos.y > 0 && m_Blocks[getBlockIndex({pos.x, pos.y-1, pos.z})] == BLOCK_TYPE::AIR)
        uncoveredFaces |= 1 << BOTTOM_FACE_INDEX;
    if (pos.y == MAX_HEIGHT-1 || m_Blocks[getBlockIndex({pos.x, pos.y+1, pos.z})] == BLOCK_TYPE::AIR)
        uncoveredFaces |= 1 << TOP_FACE_INDEX;
    if (pos.z == 0 || m_Blocks[getBlockIndex({pos.x, pos.y, pos.z-1})] == BLOCK_TYPE::AIR)
        uncoveredFaces |= 1 << FRONT_FACE_INDEX;
    if (pos.z == CHUNK_SIZE-1 || m_Blocks[getBlockIndex({pos.x, pos.y, pos.z+1})] == BLOCK_TYPE::AIR)
        uncoveredFaces |= 1 << BACK_FACE_INDEX;

    return uncoveredFaces;
}

void Chunk::selectFaces(std::vector<GLuint>& buffer)
{
    GLsizei faceCount = 0;

    for (unsigned int x = 0; x < CHUNK_SIZE; x++)
    {
        for (unsigned int y = 0; y < MAX_HEIGHT; y++)
        {
            for (unsigned int z = 0; z < CHUNK_SIZE; z++)
            {
                const glm::uvec3 pos = {x,y,z};
                const auto& block = m_Blocks[getBlockIndex(pos)];
                if (block == BLOCK_TYPE::AIR)
                    continue;

                const unsigned int uncoveredFaces = getUncoveredFaces(pos);
                if (uncoveredFaces == 0)
                    continue;

                unsigned int additionalVertData = 0;

                const glm::uvec2 atlasOffset = getAtlasOffset(block);
                additionalVertData |= atlasOffset.x << 10;
                additionalVertData |= atlasOffset.y << 11;
                additionalVertData |= x << 15;
                additionalVertData |= y << 19;
                additionalVertData |= z << 27;

                for (unsigned int i = 0; i < 6; i++)
                {
                    if ((uncoveredFaces >> i) & 1 == 0)
                        continue;

                    buffer.push_back(BLOCK_FACES[4*i+0] | additionalVertData);
                    buffer.push_back(BLOCK_FACES[4*i+1] | additionalVertData);
                    buffer.push_back(BLOCK_FACES[4*i+2] | additionalVertData);

                    buffer.push_back(BLOCK_FACES[4*i+0] | additionalVertData);
                    buffer.push_back(BLOCK_FACES[4*i+2] | additionalVertData);
                    buffer.push_back(BLOCK_FACES[4*i+3] | additionalVertData);

                    faceCount++;
                }
            }
        }
    }

    m_VAO.setVertexCount(faceCount * 6);
}