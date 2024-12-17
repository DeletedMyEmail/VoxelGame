#include "../include/Log.h"
#include "../include/Window.h"
#include "../include/Model.h"
#include "../include/Shader.h"
#include "../include/Texture.h"

static constexpr int BLOCK_COUNT = 10;

int main() {
    Log::init();
    const Window window = WindowBuilder().
                        size(1200, 1900).
                        title("Minecraft Clone").
                        disableCursor().
                        build();

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
    while (window.isRunning()) {
        window.clear();

        shader.bind();
        blockModel.vao().bind();
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, BLOCK_COUNT);
    }

    return 0;
}
