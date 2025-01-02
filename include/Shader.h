#pragma once

#include <string>
#include "glad/glad.h"
#include "glm/fwd.hpp"

class Shader {
public :
    Shader(const char* pVertexShaderSource, const char* pFragmentShaderSource, const char* pGeometryShaderSource = nullptr);
    ~Shader();

    void setUniform1f(const std::string& name, float value) const;
    void setUniform4f(const std::string& name, const glm::mat4& mat) const;
    void setUniform1i(const std::string& name, int value) const;
    void setUniform1u(const std::string& name, unsigned int value) const;
    void setUniform2u(const std::string& name, unsigned int x, unsigned int y) const;

    void bind() const;
    static void unbind();

    GLuint getShaderID() const { return m_ProgrammID; }
private:
    int getUniformLocation(const std::string& name) const;
private:
    static GLuint compile(const char* shaderSource, GLenum shaderType);
    static std::string parse(const char* shaderSource);
    GLuint m_ProgrammID;
};
