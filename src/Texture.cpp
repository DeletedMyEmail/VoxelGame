#include "../include/Texture.h"

#include "../libs/glad/glad.h"

Texture::Texture(const char* path)
    : m_ID(0), m_FilePath(path), m_Width(0), m_Height(0), m_BPP(0)
{
    stbi_set_flip_vertically_on_load(true);
    unsigned char* buffer = stbi_load(path, &m_Width, &m_Height, &m_BPP, 4);

    glGenTextures(1, &m_ID);
    glBindTexture(GL_TEXTURE_2D, m_ID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0,  GL_RGBA8, GL_UNSIGNED_BYTE, buffer);
    unbind();

    stbi_image_free(buffer);
}

Texture::~Texture() {
    if (m_ID != 0)
        glDeleteTextures(1, &m_ID);
}

void Texture::bind(const unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_ID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
