#include "Texture.h"
#include "glad/glad.h"
#include "Log.h"
#include "Renderer.h"
#include "stb/stb_image.h"

Texture::Texture(const char* path)
    : m_ID(0), m_FilePath(path), m_Width(0), m_Height(0), m_BPP(0)
{
    stbi_set_flip_vertically_on_load(true);
    unsigned char* buffer = stbi_load(path, &m_Width, &m_Height, &m_BPP, 4);

    GLCall(glGenTextures(1, &m_ID))
    GLCall(glBindTexture(GL_TEXTURE_2D, m_ID))

    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE))
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE))
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR))
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))

    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0,  GL_RGBA, GL_UNSIGNED_BYTE, buffer))

    stbi_image_free(buffer);
}

Texture::~Texture() {
    if (m_ID != 0)
    {
        GLCall(glDeleteTextures(1, &m_ID))
    }
}

void Texture::bind(const unsigned int slot) const {
    GLCall(glActiveTexture(GL_TEXTURE0 + slot))
    GLCall(glBindTexture(GL_TEXTURE_2D, m_ID))
}

void Texture::unbind() const {
    GLCall(glBindTexture(GL_TEXTURE_2D, 0))
}
