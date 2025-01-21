#pragma once
#include <glm/vec2.hpp>

enum class TextureType : unsigned char {
    TEST,
    GRASS,
    STONE,
};

inline glm::uvec2 getAtlasOffset(const TextureType texture)
{
    switch (texture)
    {
        case TextureType::GRASS:
            return {1,0};
        case TextureType::STONE:
            return {1,1};
        case TextureType::TEST:
        default:
            return {0,0};
    }
}

enum BlockType : unsigned char {
    SOLID,
    TRANSPARENT,
    AIR
};

struct Blockdata
{
    unsigned char data;

    void setType(const BlockType type)
    {
        data = data | static_cast<unsigned char>(type);
    }

    void setTexture(const TextureType texture)
    {
        data = data | (static_cast<unsigned char>(texture) << 4);
    }

    BlockType getBlockType() const
    {
        return static_cast<BlockType>(data & 0b1111);
    }

    TextureType getTextureType() const
    {
        return static_cast<TextureType>((data >> 4) & 0b1111);
    }
};