#pragma once

#include <memory>
#include <vector>
#include "Buffer.h"

struct LayoutElement
{
    GLint count;
    GLenum type;
    GLboolean normalized;
    GLsizei instanceDivisor;
};

unsigned int sizeOfGLType(GLenum type);

class VertexBufferLayout
{
public:
    VertexBufferLayout() = default;

    template<typename T>
    void push(GLint count, GLboolean normalized = GL_FALSE, GLsizei instanceDivisor = 0) {}

    const std::vector<LayoutElement>& getAttributes() const { return m_LayoutElements; }
    GLsizei getStride() const { return m_Stride; }
private:
    std::vector<LayoutElement> m_LayoutElements;
    GLsizei m_Stride = 0;
};

class VertexArray
{
public:
    VertexArray();
    ~VertexArray();

    void addBuffer(const std::shared_ptr<VertexBuffer>& buffer, const VertexBufferLayout& layout);
    void bind() const;
    void unbind() const;
    void clear();

    void setVertexCount(const GLsizei count) { m_VertexCount = count; }
    GLsizei getVertexCount() const { return m_VertexCount; }
    GLuint getAttribCount() const { return m_AttribCounter; }
    GLuint getBufferCount() const { return m_Buffers.size(); }
private:
    void setAttributes(GLuint& counter, const VertexBufferLayout& layout, bool enable) const;
private:
    std::vector<std::shared_ptr<VertexBuffer>> m_Buffers;
    GLuint m_ArrayID = 0;
    GLuint m_AttribCounter = 0;
    GLsizei m_VertexCount = 0;
};

template<>
void VertexBufferLayout::push<GLfloat>(GLint count, GLboolean normalized, GLsizei instanceDivisor);
template<>
void VertexBufferLayout::push<GLuint>(GLint count, GLboolean normalized, GLsizei instanceDivisor);