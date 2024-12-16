#pragma once
#include <string>
#include "../libs/glad/glad.h"

class Shader {
public :
    Shader(const char* pVertexShaderSource, const char* pFragmentShaderSource);
    ~Shader();

    void setUniform1f(const std::string& name, float value) const;
    void setUniform1i(const std::string& name, int value) const;
    void bind() const;
    static void unbind();

    GLuint getShaderID() const { return m_ShaderProgram; }
private:
    int getUniformLocation(const std::string& name) const;
private:
    static GLuint compile(const char* pShaderSource, GLenum pShaderType);
    static std::string parse(const char* pShaderSource);
    GLuint m_ShaderProgram;
};