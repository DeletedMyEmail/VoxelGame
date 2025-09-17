#pragma once

#include <vector>

#include "cstmlib/Log.h"
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

    VertexArray(const VertexArray& other) = delete;
    VertexArray& operator=(const VertexArray& other) = delete;

    VertexArray(VertexArray&& other) noexcept;
    VertexArray& operator=(VertexArray&& other) noexcept;

    void addBuffer(GLuint bufferId, const VertexBufferLayout& layout);
    void bind() const;
    void unbind() const;
    void reset();
    void clear();

    std::vector<GLuint> buffers;
    GLuint arrayID = 0;
    GLuint attribCounter = 0;
    GLuint vertexCount = 0;
};