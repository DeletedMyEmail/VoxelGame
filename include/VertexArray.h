#pragma once

#include <vector>
#include "glad/glad.h"

GLuint createBuffer(const GLvoid* data, GLsizei size, GLenum target = GL_ARRAY_BUFFER, GLenum usage = GL_STATIC_DRAW);

struct LayoutElement
{
    GLint count;
    GLenum type;
    GLboolean normalized;
    GLsizei instanceDivisor;
};

struct VertexBufferLayout
{
    void pushFloat(GLint count, GLboolean normalized = GL_FALSE, GLsizei instanceDivisor = 0);
    void pushUInt(GLint count, GLboolean normalized = GL_FALSE, GLsizei instanceDivisor = 0);
    void pushInt(GLint count, GLboolean normalized = GL_FALSE, GLsizei instanceDivisor = 0);

    std::vector<LayoutElement> layoutElements;
    GLsizei stride = 0;
};

struct VertexArray
{
    VertexArray();
    ~VertexArray();

    void addBuffer(GLuint bufferId, const VertexBufferLayout& layout);
    void bind() const;
    void unbind() const;
    void clear();

    std::vector<GLuint> buffers;
    GLuint arrayID = 0;
    GLuint attribCounter = 0;
    GLsizei vertexCount = 0;
};