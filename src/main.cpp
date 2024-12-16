#include "../include/Log.h"
#include "../include/Shader.h"
#include "../libs/glad/glad.h"
#include "../libs/glm/glm.hpp"
#include <GLFW/glfw3.h>

constexpr bool FULLSCREEN = false;

int main() {
    Log::init();

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    if (FULLSCREEN) {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        window = glfwCreateWindow(mode->width, mode->height, "A Simulation", glfwGetPrimaryMonitor(), NULL);
    }
    else {
        window = glfwCreateWindow(1800, 1000, "A cube game", nullptr, NULL);
    }

    if (!window) return -1;

    glfwMakeContextCurrent(window);
    gladLoadGL();

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) return -1;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSwapInterval(0); // disable vsync

    const float mesh[] = {
        // positions     // colors
        -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
         0.05f,  0.05f,  0.0f, 1.0f, 0.0f,
        -0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

         0.05f,  -0.05f,  1.0f, 0.0f, 0.0f,
         0.05f,  0.05f,  0.0f, 1.0f, 0.0f,
        -0.05f, -0.05f,  0.0f, 0.0f, 1.0f,
    };

    glm::vec2 translations[100];
    int index = 0;
    float offset = 0.1f;
    for(int y = -10; y < 10; y += 2)
    {
        for(int x = -10; x < 10; x += 2)
        {
            glm::vec2 translation;
            translation.x = (float)x / 10.0f + offset;
            translation.y = (float)y / 10.0f + offset;
            translations[index++] = translation;
        }
    }

    const Shader shader("../shader/BasicVert.glsl", "../shader/BasicFrag.glsl");

    GLuint vertArray = 0;
    GLuint vertBuffer = 0;
    GLuint instanceBuffer = 0;

    glGenVertexArrays(1, &vertArray);
    glGenBuffers(1, &vertBuffer);
    glGenBuffers(1, &instanceBuffer);

    glBindVertexArray(vertArray);

    glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*5*6, mesh, GL_STATIC_DRAW);

    // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, nullptr);
    // color attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, reinterpret_cast<void*>(sizeof(glm::vec2)));
    // translation attribute
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*100, translations, GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
    glVertexAttribDivisor(2, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();
        glClearColor(0.07f, 0.14f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.bind();
        glBindVertexArray(vertArray);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);
    }

    return 0;
}