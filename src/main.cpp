#include "Log.h"
#include "Window.h"
#include "Model.h"
#include "Shader.h"
#include "Texture.h"
#include "Renderer.h"

//static constexpr int BLOCK_COUNT = 10;

Model createBlockModel();

int main() {
    Log::init();
    const Window window = WindowBuilder().
                        size(1200, 1900).
                        title("Minecraft Clone").
                        //disableCursor().
                        onClose([](Window*) {exit(0);}).
                        build();

    const Shader shader("../shader/BasicVert.glsl", "../shader/BasicFrag.glsl");
    shader.bind();
    shader.setUniform1i("u_Texture", 0);

    const Model model = createBlockModel();

    /*GLuint instanceBuffer = 0;
    // instance rendering
    glm::vec2 translations[BLOCK_COUNT];
    for(int i = 0; i < BLOCK_COUNT; i++)
    {
        translations[i] = {10.0f * i, 10.0f * i};
    }
    glGenBuffers(1, &instanceBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * BLOCK_COUNT, translations, GL_STATIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
    glVertexAttribDivisor(2, 1);*/

    // main loop
    LOG_INFO("Starting Game");
    while (window.isRunning()) {
        window.clear();
        model.bind();
        shader.bind();

        GLCall(glDrawElements(GL_TRIANGLES, model.getIndexBuffer()->getCount(), GL_UNSIGNED_INT, nullptr))
    }

    return 0;
}

Model createBlockModel()
{
    const float verticies[] = {
        // positions          // texture coords
        -0.05f, -0.05f, 0.0,  -0.05f, -0.05f,
        -0.05f,  0.05f, 0.0,  -0.05f,  0.05f,
         0.05f, -0.05f, 0.0,   0.05f, -0.05f,
         0.05f,  0.05f, 0.0,   0.05f,  0.05f,
    };

    const GLsizei indices[] = {
        0, 1, 2, 1, 2, 3
    };

    const std::shared_ptr<VertexBuffer> vBuffer = std::make_unique<VertexBuffer>(sizeof(float) * 5 * 4, verticies);
    const std::shared_ptr<IndexBuffer> iBuffer = std::make_unique<IndexBuffer>(indices, 6);

    VertexBufferLayout layout;
    layout.push<float>(3);
    layout.push<float>(2);

    const std::shared_ptr<VertexArray> vArray = std::make_unique<VertexArray>();
    vArray->addBuffer(vBuffer, layout);

    const std::shared_ptr<Texture> texture = std::make_unique<Texture>("../resources/block.png");

    return Model(texture, vArray, iBuffer);
}