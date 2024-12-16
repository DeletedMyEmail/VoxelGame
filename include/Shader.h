#pragma once
#include <string>
#include "../libs/glad/glad.h"

class Shader {
public :
    Shader(const char* pVertexShaderSource, const char* pFragmentShaderSource);
    ~Shader();
    void bind() const;
    static void unbind();

    GLuint getShaderId() const;
private:
    static GLuint compile(const char* pShaderSource, GLenum pShaderType);
    static std::string parse(const char* pShaderSource);
    GLuint m_ShaderProgram;
};