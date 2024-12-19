#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"

class VertexBuffer {
public:
    VertexBuffer(GLsizei size, const GLvoid* data, GLenum usage = GL_STATIC_DRAW);
    ~VertexBuffer();

    void bind() const;
    void unbind();
private:
    GLuint m_BufferID = 0;
};

class IndexBuffer {
public:
    IndexBuffer(const GLsizei* indices, GLsizei count, GLenum usage = GL_STATIC_DRAW);
    ~IndexBuffer();

    void bind() const;
    void unbind() const;

    GLsizei getCount() const { return m_Count; };
private:
    GLsizei m_Count;
    GLuint m_BufferID = 0;
};