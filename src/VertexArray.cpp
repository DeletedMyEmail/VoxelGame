#include "VertexArray.h"
#include "Renderer.h"

unsigned int sizeOfGLType(const GLenum type)
{
    switch (type) {
    case GL_FLOAT:
        return sizeof(GLfloat);
    case GL_INT:
        return sizeof(GLint);
    case GL_UNSIGNED_INT:
        return sizeof(GLuint);
    case GL_BYTE:
        return sizeof(GLbyte);
    default:
        return 0;
    }
}

template <>
void VertexBufferLayout::push<GLfloat>(const GLint count, const GLboolean normalized, const GLsizei instanceDivisor)
{
    m_LayoutElements.emplace_back(LayoutElement{count, GL_FLOAT, normalized, instanceDivisor});
    m_Stride += count * sizeof(GLfloat);
}

template <>
void VertexBufferLayout::push<GLuint>(const GLint count, const GLboolean normalized, const GLsizei instanceDivisor)
{
    m_LayoutElements.emplace_back(LayoutElement{count, GL_UNSIGNED_INT, normalized, instanceDivisor});
    m_Stride += count * sizeof(GLuint);
}

VertexArray::VertexArray()
    : m_ArrayID(0)
{
    GLCall(glGenVertexArrays(1, &m_ArrayID))
}

void VertexArray::setAttributes(GLuint& counter, const VertexBufferLayout& layout, const bool enable) const
{
    bind();

    const auto& attributes = layout.getAttributes();
    unsigned int offset = 0;

    for (auto [count, type, normalized, instanceDivisor] : attributes)
    {
        if (enable)
        {
            GLCall(glEnableVertexAttribArray(counter))
        }

        if (type == GL_UNSIGNED_INT || type == GL_INT)
        {
            GLCall(glVertexAttribIPointer(counter, count, type, layout.getStride(), (void*) offset))
        }
        else
        {
            GLCall(glVertexAttribPointer(counter, count, type, normalized, layout.getStride(), (void*) offset))
        }

        if (instanceDivisor != 0)
        {
            GLCall(glVertexAttribDivisor(counter, instanceDivisor))
        }

        counter++;
        offset += sizeOfGLType(type) * count;
    }
}

void VertexArray::updateBuffer(GLuint index, const std::shared_ptr<VertexBuffer>& buffer, const VertexBufferLayout& layout)
{
    m_Buffers[index] = buffer;
    buffer->bind();
    setAttributes(index, layout, false);
}

void VertexArray::addBuffer(const std::shared_ptr<VertexBuffer>& buffer, const VertexBufferLayout& layout)
{
    m_Buffers.emplace_back(buffer);
    buffer->bind();
    setAttributes(m_AttribCounter, layout, true);
}

VertexArray::~VertexArray()
{
    if (m_ArrayID != 0)
    {
        GLCall(glDeleteVertexArrays(1, &m_ArrayID))
    }
}

void VertexArray::bind() const
{
    GLCall(glBindVertexArray(m_ArrayID))
}

void VertexArray::unbind() const
{
    GLCall(glBindVertexArray(0))
}

void VertexArray::clear()
{
    if (m_ArrayID != 0)
    {
        GLCall(glDeleteVertexArrays(1, &m_ArrayID))
    }

    m_Buffers.clear();
    m_AttribCounter = 0;
    m_ArrayID = 0;
}
