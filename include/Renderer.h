#pragma once

#include "../libs/glad/glad.h"
#include "Log.h"

#ifndef RELEASE
    #define GLCall(x) x; checkOpenGLErrors();
#else
    #define GLCall(x) x;
#endif


inline void checkOpenGLErrors() {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        LOG_ERROR(error);
    }
}