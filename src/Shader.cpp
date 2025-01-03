#include "Shader.h"
#include "Log.h"
#include "Renderer.h"
#include <fstream>
#include <sstream>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const char* pVertexShaderSource, const char* pFragmentShaderSource, const char* pGeometryShaderSource)
    : m_ProgrammID(glCreateProgram())
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

    GLCall(glAttachShader(m_ProgrammID, vertShaderID))
    GLCall(glAttachShader(m_ProgrammID, fragShaderID))
    if (pGeometryShaderSource != nullptr)
    {
        GLCall(glAttachShader(m_ProgrammID, geoShaderID))
    }

    GLCall(glLinkProgram(m_ProgrammID))

    GLCall(glDeleteShader(vertShaderID))
    GLCall(glDeleteShader(fragShaderID))
    if (pGeometryShaderSource != nullptr)
    {
        GLCall(glDeleteShader(geoShaderID))
    }

    unbind();
}

Shader::~Shader()
{
    GLCall(glDeleteProgram(m_ProgrammID))
}

void Shader::setUniform1f(const std::string &name, const float value) const
{
    bind();
    GLCall(glUniform1f(getUniformLocation(name), value))
}

void Shader::setUniform4f(const std::string& name, const glm::mat4& mat) const
{
    bind();
    GLCall(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat)));
}

void Shader::setUniform1i(const std::string &name, const int value) const
{
    bind();
    GLCall(glUniform1i(getUniformLocation(name), value))
}

void Shader::setUniform1u(const std::string &name, const unsigned int value) const
{
    bind();
    GLCall(glUniform1ui(getUniformLocation(name), value))
}

void Shader::setUniform2u(const std::string &name, const unsigned int x,const unsigned int y) const
{
    bind();
    GLCall(glUniform2ui(getUniformLocation(name), x, y))
}

int Shader::getUniformLocation(const std::string &name) const
{
    const int location = glGetUniformLocation(m_ProgrammID, name.c_str());
    if (location == -1)
        LOG_WARN("Uniform not found: " + name);
    return location;
}

void Shader::bind() const
{
    GLCall(glUseProgram(m_ProgrammID))
}

void Shader::unbind() { glUseProgram(0); }

GLuint Shader::compile(const char* shaderSource, const GLenum shaderType)
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

        LOG_ERROR(std::string("Shader compilation error: ")+message);
        exit(-1);
    }

    return shaderID;
}

std::string Shader::parse(const char* shaderSource)
{
    std::ifstream lShaderFile(shaderSource);

    if (!lShaderFile.is_open())
    {
        LOG_ERROR(std::string("Shader file not found: ") + shaderSource);
        exit(-1);
    }

    std::stringstream shaderBuffer;
    shaderBuffer << lShaderFile.rdbuf();

    return shaderBuffer.str();
}