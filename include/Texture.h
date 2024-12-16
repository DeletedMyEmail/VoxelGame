#pragma once

#include <string>
#include "../libs/stb/stb_image.h"

class Texture
{
    public:
      explicit Texture(const char* path);
      ~Texture();

      void bind(unsigned int slot = 0) const;
      void unbind() const;
    private:
      unsigned int m_ID;
      const char* m_FilePath;
      int m_Width, m_Height, m_BPP;
};