#include "../include/Log.h"
#include "../include/Model.h"
#include "../include/Shader.h"
#include "../include/Texture.h"
#include "../libs/glad/glad.h"
#include "../libs/glm/glm.hpp"
#include <GLFW/glfw3.h>

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {}
GLFWwindow* createWindow(bool fullscreen);

static constexpr int BLOCK_COUNT = 10;

int main() {
    Log::init();
    GLFWwindow* window = createWindow(false);
    if (!window) {
        LOG_ERROR("Window creation error");
        return -1;
    }

    const Model blockModel("../resources/untitled.obj");
    const Texture grasTexture("../resources/atlas.png");
    const Shader shader("../shader/BasicVert.glsl", "../shader/BasicFrag.glsl");
    shader.bind();
    shader.setUniform1i("u_Texture", 0);

    // instance rendering
    glm::vec2 translations[BLOCK_COUNT];
    for(int i = 0; i < BLOCK_COUNT; i++)
    {
        translations[i] = {10.0f * i, 10.0f * i};
    }

    blockModel.vao().bind();

    GLuint instanceBuffer = 0;
    glGenBuffers(1, &instanceBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * BLOCK_COUNT, translations, GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
    glVertexAttribDivisor(2, 1);

    // main loop
    LOG_INFO("Starting Game");
    while (!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();
        glClearColor(0.07f, 0.14f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.bind();
        blockModel.vao().bind();
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, BLOCK_COUNT);
    }

    return 0;
}

GLFWwindow* createWindow(bool fullscreen) {
    if (!glfwInit()) return nullptr;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    if (fullscreen) {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        window = glfwCreateWindow(mode->width, mode->height, "A Simulation", glfwGetPrimaryMonitor(), nullptr);
    }
    else {
        window = glfwCreateWindow(1800, 1000, "A cube game", nullptr, nullptr);
    }

    if (!window) return nullptr;

    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    gladLoadGL();

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) return nullptr;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSwapInterval(0); // disable vsync

    return window;
}
