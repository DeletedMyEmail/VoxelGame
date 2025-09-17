#include "Block.h"

blockdata packBlockData(const glm::uvec3& positionInChunk, const glm::uvec2& atlasOffset, const FACE face)
{
    constexpr uint32_t
        FACE_MASK = 0xFu, FACE_OFFSET = 28u,
        XPOS_MASK = 0x1Fu, XPOS_OFFSET = 23u,
        YPOS_MASK = 0x1Fu, YPOS_OFFSET = 18u,
        ZPOS_MASK = 0x1Fu, ZPOS_OFFSET = 13u,
        ATLASX_MASK = 0xFu, ATLASX_OFFSET = 9u,
        ATLASY_MASK = 0xFu, ATLASY_OFFSET = 5u;

    return  ((face & FACE_MASK) << FACE_OFFSET) |
            ((positionInChunk.x & XPOS_MASK) << XPOS_OFFSET) |
            ((positionInChunk.y & YPOS_MASK) << YPOS_OFFSET) |
            ((positionInChunk.z & ZPOS_MASK) << ZPOS_OFFSET) |
            ((atlasOffset.x & ATLASX_MASK) << ATLASX_OFFSET) |
            ((atlasOffset.y & ATLASY_MASK) << ATLASY_OFFSET);
}

glm::uvec2 getAtlasOffset(const BLOCK_TYPE block, const FACE face)
{
    switch (block)
    {
    case BLOCK_TYPE::TEST:
        return {0+face,0};
    case BLOCK_TYPE::GRASS:
        return {0+face,1};
    case BLOCK_TYPE::GRASS_FULL:
        return {6+face,1};
    case BLOCK_TYPE::STONE:
        return {0+face,2};
    case BLOCK_TYPE::SAND:
        return {6+face,2};
    case BLOCK_TYPE::WOOD:
        return {0+face,3};
    case BLOCK_TYPE::LEAVES:
        return {0+face,4};
    case BLOCK_TYPE::PUMPKIN:
        return {6+face,3};
    case BLOCK_TYPE::MELON:
        return {6+face,4};
    case BLOCK_TYPE::HIGHLIGHTED:
        return {15,0};
    case BLOCK_TYPE::WATER:
        return {0+face,15};
    default:
        LOG_ERROR("Invalid block type for atlas offset");
        exit(1);
    }
}

bool isTranslucent(const BLOCK_TYPE block)
{
    return block == BLOCK_TYPE::LEAVES || block == BLOCK_TYPE::WATER;
}

bool isSolid(const BLOCK_TYPE block)
{
    return block != BLOCK_TYPE::AIR && block != BLOCK_TYPE::WATER;
}
