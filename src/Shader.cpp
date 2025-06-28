#include "Shader.h"
#include <fstream>
#include <sstream>
#include "OpenGLHelper.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

int getUniformLocation(const GLuint shaderId, const std::string& name);
static GLuint compile(const char* shaderSource, GLenum shaderType);
static std::string parse(const char* shaderSource);

GLuint createShader(const char* pVertexShaderSource, const char* pFragmentShaderSource, const char* pGeometryShaderSource)
{
    const GLuint shaderId = glCreateProgram();
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

    GLCall(glAttachShader(shaderId, vertShaderID))
    GLCall(glAttachShader(shaderId, fragShaderID))
    if (pGeometryShaderSource != nullptr)
        GLCall(glAttachShader(shaderId, geoShaderID))

    GLCall(glLinkProgram(shaderId))

    GLCall(glDeleteShader(vertShaderID))
    GLCall(glDeleteShader(fragShaderID))
    if (pGeometryShaderSource != nullptr)
        GLCall(glDeleteShader(geoShaderID))

    unbindShader();
    return shaderId;
}

void deleteShader(const GLuint shaderId)
{
    GLCall(glDeleteProgram(shaderId));
}

void setUniform1f(const GLuint shaderId, const std::string &name, const float value)
{
    GLCall(glUniform1f(getUniformLocation(shaderId, name), value))
}

void setUniformMat4(const GLuint shaderId, const std::string& name, const glm::mat4& mat)
{
    GLCall(glUniformMatrix4fv(getUniformLocation(shaderId, name), 1, GL_FALSE, glm::value_ptr(mat)));
}

void setUniform3f(const GLuint shaderId, const std::string& name, const glm::vec3& vec)
{
    GLCall(glUniform3f(getUniformLocation(shaderId, name), vec.x, vec.y, vec.z));
}

void setUniform1i(const GLuint shaderId, const std::string &name, const int value)
{
    GLCall(glUniform1i(getUniformLocation(shaderId, name), value))
}

void setUniform1u(const GLuint shaderId, const std::string &name, const unsigned int value)
{
    GLCall(glUniform1ui(getUniformLocation(shaderId, name), value))
}

void setUniform2u(const GLuint shaderId, const std::string &name, const unsigned int x,const unsigned int y)
{
    GLCall(glUniform2ui(getUniformLocation(shaderId, name), x, y))
}

int getUniformLocation(const GLuint shaderId, const std::string &name)
{
    const int location = glGetUniformLocation(shaderId, name.c_str());
    if (location == -1)
        LOG_WARN("Uniform not found: {}", name);
    return location;
}

void bind(const GLuint shaderId) { GLCall(glUseProgram(shaderId)); }

void unbindShader() { GLCall(glUseProgram(0)); }

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