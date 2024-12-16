#pragma once

#include "../libs/glad/glad.h"
#include "../libs/glm/glm.hpp"

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec4 color;
    glm::vec2 texture;
};

class VAO
{
    public:
        VAO(const Vertex* pVertices, GLsizeiptr size);
        ~VAO();

        void bind() const;
        static void unbind();

        GLsizeiptr size() const { return m_Size; };
    private:
        GLsizeiptr m_Size;
        GLuint m_BufferID = 0, m_VaoID = 0;
};

class Model
{
    public:
        explicit Model(const char* objPath);
        ~Model();

        void rotate(float pDegrees, glm::vec3 pAxis);
        void scale(glm::vec3 pScale);
        void translate(const glm::vec3& pTranslation);

        const VAO& vao() const { return m_VAO; }
        const glm::mat4& getMatrix() const { return m_Matrix; }
        const glm::mat4& getViewProj() const { return m_ViewProjection; }
    private:
        static VAO parse(const char* objPath);
    private:
        VAO m_VAO;
        glm::mat4 m_Matrix;
        glm::mat4 m_ViewProjection;
};
