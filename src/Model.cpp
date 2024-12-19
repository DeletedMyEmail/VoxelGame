#include "../include/Model.h"
#include "../libs/glm/gtc/matrix_transform.hpp"

Model::Model(const std::shared_ptr<Texture>& texture, const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<IndexBuffer>& indexBuffer)
    : m_Texture(texture), m_IndexBuffer(indexBuffer), m_VertArray(vertexArray), m_Matrix(), m_ViewProjection()
{
}

Model::~Model() = default;

void Model::rotate(const float degrees, const glm::vec3& axis)
{
    m_Matrix = glm::rotate(m_Matrix, glm::radians(degrees), axis);
}

void Model::scale(const glm::vec3& scale)
{
    m_Matrix = glm::scale(m_Matrix, scale);
}

void Model::translate(const glm::vec3& translation)
{
    m_Matrix = glm::translate(m_Matrix, translation);
}

void Model::bind() const
{
    m_IndexBuffer->bind();
    m_VertArray->bind();
}

void Model::unbind() const
{
    m_IndexBuffer->unbind();
    m_VertArray->unbind();
}