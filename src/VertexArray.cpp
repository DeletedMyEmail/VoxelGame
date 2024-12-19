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
void VertexBufferLayout::push<float>(const GLint count, const GLboolean normalized, const GLsizei instanceDivisor)
{
    m_LayoutElements.emplace_back(LayoutElement{count, GL_FLOAT, normalized, instanceDivisor});
    m_Stride += count * sizeof(GLfloat);
}

VertexArray::VertexArray()
    : m_ArrayID(0)
{
    GLCall(glGenVertexArrays(1, &m_ArrayID))
}

void VertexArray::addBuffer(const std::shared_ptr<VertexBuffer>& buffer, const VertexBufferLayout& layout)
{
    bind();
    buffer->bind();
    m_Buffers.emplace_back(buffer);

    const auto& attributes = layout.getAttributes();
    unsigned int offset = 0;

    for (unsigned int i = 0; i < attributes.size(); i++)
    {
        auto [count, type, normalized, instanceDivisor] = attributes.at(i);

        GLCall(glEnableVertexAttribArray(i))
        GLCall(glVertexAttribPointer(i, count, type, normalized, layout.getStride(), (void*) offset))
        if (instanceDivisor != 0)
        {
            GLCall(glVertexAttribDivisor(i, instanceDivisor))
        }

        offset += sizeOfGLType(type);
    }
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