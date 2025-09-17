#pragma once

#include "cstmlib/Log.h"
#include "glad/glad.h"

#ifndef RELEASE
    #define GLCall(x) {x; checkOpenGLErrors();}
#else
    #define GLCall(x) x;
#endif

inline void checkOpenGLErrors()
{
    GLenum error = GL_NO_ERROR;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        LOG_ERROR("OpenGL error: {}", std::to_string(error));
    }
}
