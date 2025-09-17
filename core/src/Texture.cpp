#include "Texture.h"

#include "OpenGLHelper.h"
#include "cstmlib/Log.h"
#include "glad/glad.h"
#include "stb/stb_image.h"

Texture::Texture(const char* path)
    : m_id(0), m_filePath(path), m_width(0), m_height(0), m_bpp(0)
{
    stbi_set_flip_vertically_on_load(false);
    unsigned char* buffer = stbi_load(path, &m_width, &m_height, &m_bpp, 4);
    if (!buffer)
    {
        LOG_ERROR("Failed to load texture");
        return;
    }

    GLCall(glGenTextures(1, &m_id))
    GLCall(glBindTexture(GL_TEXTURE_2D, m_id))

    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST))
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST))
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE))
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE))

    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0,  GL_RGBA, GL_UNSIGNED_BYTE, buffer))

    stbi_image_free(buffer);
}

Texture::~Texture() {
    if (m_id != 0)
    {
        GLCall(glDeleteTextures(1, &m_id))
    }
}

void Texture::bind(const unsigned int slot) const {
    GLCall(glActiveTexture(GL_TEXTURE0 + slot))
    GLCall(glBindTexture(GL_TEXTURE_2D, m_id))
}

void Texture::unbind() const {
    GLCall(glBindTexture(GL_TEXTURE_2D, 0))
}
