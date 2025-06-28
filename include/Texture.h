#pragma once

struct Texture
{
    Texture(const char* path);
    ~Texture();
    void bind(unsigned int slot = 0) const;
    void unbind() const;

    unsigned int m_id;
    const char* m_filePath;
    int m_width, m_height, m_bpp;
};