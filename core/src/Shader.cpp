#include "Shader.h"
#include <fstream>
#include <sstream>
#include "OpenGLHelper.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

int getUniformLocation(const GLuint shaderId, const std::string& name);
static GLuint compile(const char* shaderSource, GLenum shaderType);
static std::string parse(const char* shaderSource);

Shader::Shader(const char* pVertexShaderSource, const char* pFragmentShaderSource, const char* pGeometryShaderSource)
    : m_ID(glCreateProgram())
{
    const std::string vertStr = parse(pVertexShaderSource);
    const GLuint vertShaderID = compile(vertStr.c_str(), GL_VERTEX_SHADER);

    const std::string fragStr = parse(pFragmentShaderSource);
    const GLuint fragShaderID = compile(fragStr.c_str(), GL_FRAGMENT_SHADER);

    GLuint geoShaderID = 0;
    if (pGeometryShaderSource != nullptr)
    {
        const std::string geoStr = parse(pGeometryShaderSource);
        geoShaderID = compile(geoStr.c_str(), GL_GEOMETRY_SHADER);
    }

    GLCall(glAttachShader(m_ID, vertShaderID))
    GLCall(glAttachShader(m_ID, fragShaderID))
    if (pGeometryShaderSource != nullptr)
        GLCall(glAttachShader(m_ID, geoShaderID))

    GLCall(glLinkProgram(m_ID))

    GLCall(glDeleteShader(vertShaderID))
    GLCall(glDeleteShader(fragShaderID))
    if (pGeometryShaderSource != nullptr)
        GLCall(glDeleteShader(geoShaderID))
}

Shader::~Shader()
{
    GLCall(glDeleteProgram(m_ID));
}

void Shader::setUniform1f(const std::string &name, const float value)
{
    GLCall(glUniform1f(getUniformLocation(name), value))
}

void Shader::setUniformMat4(const std::string& name, const glm::mat4& mat)
{
    GLCall(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat)));
}

void Shader::setUniform3f(const std::string& name, const glm::vec3& vec)
{
    GLCall(glUniform3f(getUniformLocation(name), vec.x, vec.y, vec.z));
}

void Shader::setUniform1i(const std::string &name, const int value)
{
    GLCall(glUniform1i(getUniformLocation(name), value))
}

void Shader::setUniform1u(const std::string &name, const unsigned int value)
{
    GLCall(glUniform1ui(getUniformLocation(name), value))
}

void Shader::setUniform2u(const std::string &name, const unsigned int x,const unsigned int y)
{
    GLCall(glUniform2ui(getUniformLocation(name), x, y))
}

GLint Shader::getUniformLocation(const std::string &name)
{
    if (m_UniformLocations.contains(name))
        return m_UniformLocations[name];

    const GLint location = glGetUniformLocation(m_ID, name.c_str());
    if (location == -1)
        LOG_WARN("Uniform not found: {}", name);
    m_UniformLocations[name] = location;

    return location;
}

void Shader::bind() const
{
    GLCall(glUseProgram(m_ID));
}

void Shader::unbind() const
{
    GLCall(glUseProgram(0));
}

GLuint compile(const char* shaderSource, const GLenum shaderType)
{
    const GLuint shaderID = glCreateShader(shaderType);

    int res = -1;
    GLCall(glShaderSource(shaderID, 1, &shaderSource, nullptr))
    GLCall(glCompileShader(shaderID))
    GLCall(glGetShaderiv(shaderID, GL_COMPILE_STATUS, &res))

    if(res != GL_TRUE)
    {
        int length = 0;
        GLCall(glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length))
        char message[length];
        GLCall(glGetShaderInfoLog(shaderID, length, &length, message))

        LOG_ERROR("Shader compilation error: {}", std::string(message));
        exit(-1);
    }

    return shaderID;
}

std::string parse(const char* shaderSource)
{
    std::ifstream lShaderFile(shaderSource);

    if (!lShaderFile.is_open())
    {
        LOG_ERROR("Shader file not found: {}", shaderSource);
        exit(-1);
    }

    std::stringstream shaderBuffer;
    shaderBuffer << lShaderFile.rdbuf();

    return shaderBuffer.str();
}