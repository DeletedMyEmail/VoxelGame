#include <fstream>
#include <string>
#include <sstream>
#include "../include/Model.h"
#include "../include/Log.h"
#include "../libs/glm/gtc/matrix_transform.hpp"

VAO::VAO(const Vertex* pVertices, const GLsizeiptr size)
    : m_Size(size)
{
    glGenVertexArrays(1, &m_VaoID);
    glGenBuffers(1, &m_BufferID);

    glBindVertexArray(m_VaoID);

    glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
    glBufferData(GL_ARRAY_BUFFER, m_Size * sizeof(Vertex), pVertices, GL_STATIC_DRAW);

    // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    // normal attribute
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
    // color attribute
    //glEnableVertexAttribArray(2);
    //glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, color)));
    // texture attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, texture)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

VAO::~VAO()
{
    if (m_VaoID != 0)
        glDeleteVertexArrays(1, &m_VaoID);
}

void VAO::bind() const
{
    glBindVertexArray(m_VaoID);
}

void VAO::unbind()
{
    glBindVertexArray(0);
}

// -------------------------------

Model::Model(const char* objPath)
    : m_VAO(parse(objPath)), m_Matrix(glm::mat4(1.0f)), m_ViewProjection(glm::mat4(1.0f))
{

}

Model::~Model()
{

}

void Model::rotate(const float pDegrees, const glm::vec3 pAxis)
{
    m_Matrix = glm::rotate(m_Matrix, glm::radians(pDegrees), pAxis);
}

void Model::scale(const glm::vec3 pScale)
{
    m_Matrix = glm::scale(m_Matrix, pScale);
}

void Model::translate(const glm::vec3& pTranslation)
{
    m_Matrix = glm::translate(m_Matrix, pTranslation);
}

static glm::uvec3 parseFace(const std::string& line);
static glm::vec3 parsePosition(const char* pLine);
static glm::vec3 parseNormal(const char* pLine);
static glm::vec2 parseTexture(const char* pLine);

VAO Model::parse(const char* objPath)
{
    std::ifstream lFile(objPath);
    if (!lFile)
    {
        LOG_ERROR("Obj file not found");
        return {nullptr,0};;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> textures;
    std::vector<glm::vec3> normals;

    std::string lLine;
    while (std::getline(lFile, lLine))
    {
        if (lLine.substr(0, 2) == "v ")
            positions.push_back(parsePosition(lLine.c_str()));

        else if (lLine.substr(0, 2) == "vn")
            normals.push_back(parseNormal(lLine.c_str()));

        else if (lLine.substr(0, 2) == "vt")
            textures.push_back(parseTexture(lLine.c_str()));

        /*else if (lLine.substr(0, 2) == "f ")
            parseFace(lLine);
        */
    }

    lFile.close();

    return {nullptr,0};
}

static glm::uvec3 parseFace(const std::string& line)
{
    std::istringstream stream(line);
    std::string token;
    glm::uvec3 face{0,0,0};

    // ignore command character ("f ")
    std::getline(stream, token, ' ');
    for (int i = 0; i < 3; i++)
    {
        std::getline(stream, token, ' ');
        if (token.find('/') != std::string::npos) {}

        // read v
        std::string v = token.substr(0, token.find('/'));
        face.x = std::stoul(v)-1;
    }

    return face;
}

static glm::vec3 parsePosition(const char* pLine)
{
    glm::vec3 v;
    sscanf(pLine, "v %f %f %f", &v.x, &v.y, &v.z);
    return v;
}

static glm::vec3 parseNormal(const char* pLine)
{
    glm::vec3 vn;
    sscanf(pLine, "vn %f %f %f", &vn.x, &vn.y, &vn.z);
    return vn;
}

static glm::vec2 parseTexture(const char* pLine)
{
    glm::vec2 vt;
    sscanf(pLine, "vt %f %f", &vt.x, &vt.y);
    return vt;
}