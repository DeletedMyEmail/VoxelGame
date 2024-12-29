#pragma once

#include "Buffer.h"
#include "Texture.h"
#include "VertexArray.h"

class Mesh
{
public:
    Mesh(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<IndexBuffer>& indexBuffer);
    ~Mesh();

    void rotate(float degrees, const glm::vec3& axis);
    void scale(const glm::vec3& scale);
    void translate(const glm::vec3& translation);
    void bind() const;
    void unbind() const;

    VertexArray* getVertexArray() const { return m_VertArray.get(); }
    IndexBuffer* getIndexBuffer() const { return m_IndexBuffer.get(); }
    const glm::mat4& getMatrix() const { return m_Matrix; }
private:
    std::shared_ptr<IndexBuffer> m_IndexBuffer;
    std::shared_ptr<VertexArray> m_VertArray;
    glm::mat4 m_Matrix;
};