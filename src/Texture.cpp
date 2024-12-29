#include "Texture.h"
#include "glad/glad.h"
#include "Log.h"
#include "Renderer.h"
#include "stb/stb_image.h"

Texture::Texture(const char* path)
    : m_ID(0), m_FilePath(path), m_Width(0), m_Height(0), m_BPP(0)
{
    stbi_set_flip_vertically_on_load(false);
    unsigned char* buffer = stbi_load(path, &m_Width, &m_Height, &m_BPP, 4);
    if (!buffer)
    {
        LOG_ERROR("Failed to load texture");
        return;
    }

    GLCall(glGenTextures(1, &m_ID))
    GLCall(glBindTexture(GL_TEXTURE_2D, m_ID))

    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST))
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST))
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER))
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER))

    float borderColor[] = {1.0f, 0.0f, 0.0f, 1.0f}; // Example: Red color
    GLCall(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor))

    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0,  GL_RGBA, GL_UNSIGNED_BYTE, buffer))

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
