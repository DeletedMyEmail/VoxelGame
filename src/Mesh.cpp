#include "Mesh.h"
#include "glm/gtc/matrix_transform.hpp"

Mesh::Mesh(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<IndexBuffer>& indexBuffer)
    : m_IndexBuffer(indexBuffer), m_VertArray(vertexArray), m_Matrix(), m_ViewProjection()
{
}

Mesh::~Mesh() = default;

void Mesh::rotate(const float degrees, const glm::vec3& axis)
{
    m_Matrix = glm::rotate(m_Matrix, glm::radians(degrees), axis);
}

void Mesh::scale(const glm::vec3& scale)
{
    m_Matrix = glm::scale(m_Matrix, scale);
}

void Mesh::translate(const glm::vec3& translation)
{
    m_Matrix = glm::translate(m_Matrix, translation);
}

void Mesh::bind() const
{
    m_IndexBuffer->bind();
    m_VertArray->bind();
}

void Mesh::unbind() const
{
    m_IndexBuffer->unbind();
    m_VertArray->unbind();
}