#pragma once

#include "Camera.h"
#include "Chunk.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"

#ifndef RELEASE
    #define GLCall(x) x; checkOpenGLErrors();
#else
    #define GLCall(x) x;
#endif

void checkOpenGLErrors();

void clear(const Window& window, glm::vec4 color = {0.07f, 0.14f, 0.17f, 1.0f}) ;
void draw(Chunk& chunk, const Shader& shader, const Texture& texture, const Window& window, const Camera& cam);