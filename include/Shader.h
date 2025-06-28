#pragma once

#include <string>
#include "glad/glad.h"
#include "glm/fwd.hpp"

GLuint createShader(const char* pVertexShaderSource, const char* pFragmentShaderSource, const char* pGeometryShaderSource = nullptr);
void deleteShader(GLuint shaderId);
void setUniform1f(GLuint shaderId, const std::string& name, float value);
void setUniformMat4(GLuint shaderId, const std::string& name, const glm::mat4& mat);
void setUniform3f(GLuint shaderId, const std::string& name, const glm::vec3& vec);
void setUniform1i(GLuint shaderId, const std::string& name, int value);
void setUniform1u(GLuint shaderId, const std::string& name, unsigned int value);
void setUniform2u(GLuint shaderId, const std::string& name, unsigned int x, unsigned int y);
void bind(GLuint shaderId);
static void unbindShader();
