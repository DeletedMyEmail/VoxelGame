#include "Buffer.h"
#include "Renderer.h"

VertexBuffer::VertexBuffer(const GLsizei size, const GLvoid* data, const GLenum usage)
    : m_BufferID(0)
{
    GLCall(glGenBuffers(1, &m_BufferID))
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_BufferID))
    GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, usage))
}

VertexBuffer::~VertexBuffer()
{
    GLCall(glDeleteBuffers(1, &m_BufferID))
}

void VertexBuffer::bind() const
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_BufferID))
}

void VertexBuffer::unbind()
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0))
}

// -------------------------------------------------------

IndexBuffer::IndexBuffer(const GLsizei* indices, const GLsizei count, const GLenum usage)
    : m_Count(count)
{
    GLCall(glGenBuffers(1, &m_BufferID))
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferID))
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices, usage))
}

IndexBuffer::~IndexBuffer() {
    GLCall(glDeleteBuffers(1, &m_BufferID))
}

void IndexBuffer::bind() const {
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferID))
}

void IndexBuffer::unbind() const {
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0))
}