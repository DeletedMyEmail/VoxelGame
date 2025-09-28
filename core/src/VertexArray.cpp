#include "VertexArray.h"
#include "OpenGLHelper.h"

GLuint createBuffer(const GLvoid* data, const GLsizei size, const GLenum target, const GLenum usage)
{
    GLuint bufferId = 0;
    GLCall(glGenBuffers(1, &bufferId));
    GLCall(glBindBuffer(target, bufferId));
    GLCall(glBufferData(target, size, data, usage));

    return bufferId;
}

static unsigned int sizeOfGLType(const GLenum type)
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

void VertexBufferLayout::pushFloat(const GLint count, const GLboolean normalized, const GLsizei instanceDivisor)
{
    layoutElements.emplace_back(LayoutElement{count, GL_FLOAT, normalized, instanceDivisor});
    stride += count * sizeof(GLfloat);
}

void VertexBufferLayout::pushUInt(const GLint count, const GLboolean normalized, const GLsizei instanceDivisor)
{
    layoutElements.emplace_back(LayoutElement{count, GL_UNSIGNED_INT, normalized, instanceDivisor});
    stride += count * sizeof(GLuint);
}

void VertexBufferLayout::pushInt(const GLint count, const GLboolean normalized, const GLsizei instanceDivisor)
{
    layoutElements.emplace_back(LayoutElement{count, GL_INT, normalized, instanceDivisor});
    stride += count * sizeof(GLint);
}

VertexArray::VertexArray()
{
    GLCall(glGenVertexArrays(1, &arrayID))
}

VertexArray::VertexArray(VertexArray&& other) noexcept
    : buffers(std::move(other.buffers)),
      arrayID(other.arrayID),
      attribCounter(other.attribCounter),
      vertexCount(other.vertexCount)
{
    other.arrayID = 0;
    other.attribCounter = 0;
    other.vertexCount = 0;
    other.buffers.clear();
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
{
    if (this == &other)
        return *this;

    clear();

    arrayID = other.arrayID;
    buffers = std::move(other.buffers);
    attribCounter = other.attribCounter;
    vertexCount = other.vertexCount;

    other.arrayID = 0;
    other.attribCounter = 0;
    other.vertexCount = 0;
    other.buffers.clear();

    return *this;
}

void VertexArray::addBuffer(const GLuint bufferId, const VertexBufferLayout& layout)
{
    buffers.emplace_back(bufferId);
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, bufferId))
    bind();

    const auto& attributes = layout.layoutElements;
    unsigned int offset = 0;

    for (auto& [count, type, normalized, instanceDivisor] : attributes)
    {
        GLCall(glEnableVertexAttribArray(attribCounter))

        if (type == GL_UNSIGNED_INT || type == GL_INT)
            GLCall(glVertexAttribIPointer(attribCounter, count, type, layout.stride, reinterpret_cast<void*>(offset)))
        else
            GLCall(glVertexAttribPointer(attribCounter, count, type, normalized, layout.stride, reinterpret_cast<void*>(offset)))

        if (instanceDivisor != 0)
            GLCall(glVertexAttribDivisor(attribCounter, instanceDivisor))

        attribCounter++;
        offset += sizeOfGLType(type) * count;
    }
}

VertexArray::~VertexArray()
{
    clear();
}

void VertexArray::bind() const
{
    GLCall(glBindVertexArray(arrayID))
}

void VertexArray::unbind() const
{
    GLCall(glBindVertexArray(0))
}

void VertexArray::reset()
{
    clear();
    GLCall(glGenVertexArrays(1, &arrayID))
}

void VertexArray::clear()
{
    if (arrayID != 0)
        GLCall(glDeleteVertexArrays(1, &arrayID))

    for (const auto& buffer : buffers)
        GLCall(glDeleteBuffers(1, &buffer))
    buffers.clear();
    attribCounter = 0;
}
