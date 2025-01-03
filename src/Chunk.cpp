#include "Chunk.h"

#include "Log.h"
#include "Noise.h"

Chunk::Chunk()
    : m_ChunkPosition({0}), m_Blocks()
{
}

Chunk::Chunk(const glm::uvec2 chunkPosition, unsigned char** heightMap)
    : m_ChunkPosition(chunkPosition), m_Blocks()
{
    createBlocks(heightMap);
    generateMesh();
}

// 32 bits - 0: pos-z, 1: pos-y, 2: pos-x, 3-5: texture-x, 6: texture-y, 7-9: normal
// (missing: 10: atlas-x, 11-14: atlas-y, 15-18: pos-rel-to-chunk-x, 19-26: pos-rel-to-chunk-y, 27-30: pos-rel-to-chunk-z, 31: unused)
static constexpr GLuint faces[] = {
    // Front Face
    0b1001001111u, // Bottom-left
    0b1001000011u, // Bottom-right
    0b1000000001u, // Top-right
    0b1000001101u, // Top-left
    // Back Face
    0b1011001110u, // Bottom-left
    0b1011010010u, // Bottom-right
    0b1010010000u, // Top-right
    0b1010001100u, // Top-left
    // Left Face
    0b0101010111u, // Bottom-left
    0b0101011110u, // Bottom-right
    0b0100011100u, // Top-right
    0b0100010101u, // Top-left
    // Right Face
    0b0111100011u, // Bottom-left
    0b0111011010u, // Bottom-right
    0b0110011000u, // Top-right
    0b0110100001u, // Top-left
    // Top Face
    0b0001101101u, // Bottom-left
    0b0001100001u, // Bottom-right
    0b0000100000u, // Top-right
    0b0000101100u, // Top-left
    // Bottom Face
    0b0011101111u, // Bottom-left
    0b0011110011u, // Bottom-right
    0b0010110010u, // Top-right
    0b0010101110u, // Top-left
};

static constexpr unsigned int FRONT_FACE_INDEX = 0;
static constexpr unsigned int BACK_FACE_INDEX = 1;
static constexpr unsigned int LEFT_FACE_INDEX = 2;
static constexpr unsigned int RIGHT_FACE_INDEX = 3;
static constexpr unsigned int TOP_FACE_INDEX = 4;
static constexpr unsigned int BOTTOM_FACE_INDEX = 5;

unsigned int Chunk::getUncoveredFaces(glm::uvec3 pos)
{
    unsigned int uncoveredFaces = 0;
    if (pos.x == 0 || m_Blocks[getBlockIndex({pos.x-1, pos.y, pos.z})].type == INVISIBLE)
        uncoveredFaces |= 1 << LEFT_FACE_INDEX;
    if (pos.x == CHUNK_SIZE-1 || m_Blocks[getBlockIndex({pos.x+1, pos.y, pos.z})].type == INVISIBLE)
        uncoveredFaces |= 1 << RIGHT_FACE_INDEX;
    if (pos.y == 0 || m_Blocks[getBlockIndex({pos.x, pos.y-1, pos.z})].type == INVISIBLE)
        uncoveredFaces |= 1 << BOTTOM_FACE_INDEX;
    if (pos.y == MAX_HEIGHT-1 || m_Blocks[getBlockIndex({pos.x, pos.y+1, pos.z})].type == INVISIBLE)
        uncoveredFaces |= 1 << TOP_FACE_INDEX;
    if (pos.z == 0 || m_Blocks[getBlockIndex({pos.x, pos.y, pos.z-1})].type == INVISIBLE)
        uncoveredFaces |= 1 << FRONT_FACE_INDEX;
    if (pos.z == CHUNK_SIZE-1 || m_Blocks[getBlockIndex({pos.x, pos.y, pos.z+1})].type == INVISIBLE)
        uncoveredFaces |= 1 << BACK_FACE_INDEX;

    return uncoveredFaces;
}

void Chunk::selectFaces(std::vector<GLuint>& buffer)
{
    for (unsigned int x = 0; x < CHUNK_SIZE; x++)
    {
        for (unsigned int y = 0; y < MAX_HEIGHT; y++)
        {
            for (unsigned int z = 0; z < CHUNK_SIZE; z++)
            {
                glm::uvec3 pos = {x,y,z};
                const auto& [type, atlasOffset] = m_Blocks[getBlockIndex(pos)];
                if (type == INVISIBLE)
                    continue;

                unsigned int uncoveredFaces = getUncoveredFaces(pos);
                if (uncoveredFaces == 0)
                    continue;

                unsigned int additionalVertData = 0;
                additionalVertData |= atlasOffset.x << 10;
                additionalVertData |= atlasOffset.y << 11;
                additionalVertData |= x << 15;
                additionalVertData |= y << 19;
                additionalVertData |= z << 27;

                for (unsigned int i = 0; i < 6; i++)
                {
                    if ((uncoveredFaces >> i) & 1 == 0)
                        continue;

                    buffer.push_back(faces[4*i+0] | additionalVertData);
                    buffer.push_back(faces[4*i+1] | additionalVertData);
                    buffer.push_back(faces[4*i+2] | additionalVertData);

                    buffer.push_back(faces[4*i+0] | additionalVertData);
                    buffer.push_back(faces[4*i+2] | additionalVertData);
                    buffer.push_back(faces[4*i+3] | additionalVertData);

                    m_FaceCount++;
                }
            }
        }
    }
}

void Chunk::generateMesh()
{
    //m_VAO.clear();

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
    LOG_INFO("Generating chunk at position: " + std::to_string(m_ChunkPosition.x * CHUNK_SIZE) + ", " + std::to_string(m_ChunkPosition.y * CHUNK_SIZE));
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
                    m_Blocks[index] = Blockdata{INVISIBLE};
                }
                else
                {
                    const auto atlasOffset = y > localHeight-3 ? GRASS_TEXTURE_OFFSET : STONE_TEXTURE_OFFSET;
                    m_Blocks[index] = Blockdata{SOLID, atlasOffset};
                }
            }
        }
    }
}

