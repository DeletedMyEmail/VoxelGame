#pragma once

#include <string>
#include <unordered_map>

#include "glad/glad.h"
#include "glm/fwd.hpp"

class Shader
{
public:
    Shader(const char* pVertexShaderSource, const char* pFragmentShaderSource, const char* pGeometryShaderSource = nullptr);
    ~Shader();
    void setUniform1f(const std::string& name, float value);
    void setUniformMat4(const std::string& name, const glm::mat4& mat);
    void setUniform3f(const std::string& name, const glm::vec3& vec);
    void setUniform1i(const std::string& name, int value);
    void setUniform1u(const std::string& name, unsigned int value);
    void setUniform2u(const std::string& name, unsigned int x, unsigned int y);
    void bind() const;
    void unbind() const;
private:
    GLint getUniformLocation(const std::string& name);
private:
    GLuint m_ID;
    std::pmr::unordered_map<std::string, GLuint> m_UniformLocations;
};
