#include "Shader.h"
#include "Log.h"
#include "Renderer.h"
#include <fstream>
#include <sstream>
#include <GLFW/glfw3.h>

Shader::Shader(const char* pVertexShaderSource, const char* pFragmentShaderSource)
    : m_ShaderProgram(glCreateProgram())
{
    const std::string lVertexString = parse(pVertexShaderSource);
    const char* lVertexCString = lVertexString.c_str();
    const GLuint lVertShader = compile(lVertexCString, GL_VERTEX_SHADER);

    const std::string lFragString = parse(pFragmentShaderSource);
    const char* lFragCString = lFragString.c_str();
    const GLuint lFragShader = compile(lFragCString, GL_FRAGMENT_SHADER);

    GLCall(glAttachShader(m_ShaderProgram, lVertShader))
    GLCall(glAttachShader(m_ShaderProgram, lFragShader))

    GLCall(glLinkProgram(m_ShaderProgram))

    GLCall(glDeleteShader(lVertShader))
    GLCall(glDeleteShader(lFragShader))

    unbind();
}

Shader::~Shader()
{
    GLCall(glDeleteProgram(m_ShaderProgram))
}

void Shader::setUniform1f(const std::string &name, float value) const {
    GLCall(glUniform1f(getUniformLocation(name), value))
}

void Shader::setUniform1i(const std::string &name, int value) const {
    GLCall(glUniform1i(getUniformLocation(name), value))
}

int Shader::getUniformLocation(const std::string &name) const {
    const int location = glGetUniformLocation(m_ShaderProgram, name.c_str());
    if (location == -1)
        LOG_WARN("Uniform not found: " + name);
    return location;
}

void Shader::bind() const
{
    GLCall(glUseProgram(m_ShaderProgram))
}

void Shader::unbind() { glUseProgram(0); }

GLuint Shader::compile(const char* pShaderSource, const GLenum pShaderType)
{
    const GLuint shaderID = glCreateShader(pShaderType);

    int compRes;
    GLCall(glShaderSource(shaderID, 1, &pShaderSource, nullptr))
    GLCall(glCompileShader(shaderID))
    GLCall(glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compRes))

    if(compRes != GL_TRUE)
    {
        int length = 0;
        GLCall(glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length))
        char message[length];
        GLCall(glGetShaderInfoLog(shaderID, length, &length, message))

        LOG_ERROR(std::string("Shader compilation error: ")+message);
        return 0;
    }

    return shaderID;
}

std::string Shader::parse(const char* shaderSource) {
    std::ifstream lShaderFile(shaderSource);

    if (!lShaderFile.is_open()) {
        LOG_ERROR(std::string("Shader file not found: ") + shaderSource);
        return "";
    }

    std::stringstream shaderBuffer;
    shaderBuffer << lShaderFile.rdbuf();

    return shaderBuffer.str();
}