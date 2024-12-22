#include "App.h"

const unsigned INSTANCE_COUNT = 100;

Mesh loadBlockMesh();

App::App()
    : m_Renderer(WindowBuilder().
                size(1200, 1900).
                title("Minecraft Clone").
                disableCursor().
                onScroll([this](Window* win, double x, double y)
                {
                    this->m_Renderer.getCamera()->increaseSpeed(y);
                }).
                onClose([](Window*)
                {
                    exit(0);
                }).
                build(),
                std::make_unique<Camera>(glm::vec3{0,0,-5}, 90.0f, 1200, 1900, 0.1f, 1000.0f))
{
}

void App::run()
{
    const std::shared_ptr<Window> window = m_Renderer.getWindow();
    const std::shared_ptr<Camera> cam = m_Renderer.getCamera();
    const Shader shader("../shader/BlockVert.glsl", "../shader/BlockFrag.glsl");
    const Mesh mesh = loadBlockMesh();
    const Texture texture("../resources/cube.png");

    auto prevMousePos = window->getMousePosition();
    float lastTime = glfwGetTime();
    float deltaSum = 0;
    unsigned frameCount = 0;

    while (window->isRunning()) {
        const float currentTime = glfwGetTime();
        const float deltaTime = currentTime - lastTime;
        deltaSum += deltaTime;
        lastTime = currentTime;
        frameCount++;

        if (deltaSum >= 1.0)
        {
            window->setTitle("Minecraft Clone  |  FPS: " + std::to_string(frameCount) + "  |  Avg frame time: " + std::to_string(deltaSum / frameCount));
            deltaSum = 0;
            frameCount = 0;
        }

        m_Renderer.update();
        m_Renderer.clear();

        if (window->isKeyDown(GLFW_KEY_ESCAPE)) exit(0);

        const auto mousePos = window->getMousePosition();
        if (mousePos != prevMousePos)
        {
            const auto relMouseMovement = mousePos - prevMousePos;

            cam->rotate(relMouseMovement.x, relMouseMovement.y);
            prevMousePos = mousePos;
        }

        glm::vec3 movement(0.0f);
        if (window->isKeyDown(GLFW_KEY_W)) movement.z += 1.0f;
        if (window->isKeyDown(GLFW_KEY_S)) movement.z -= 1.0f;
        if (window->isKeyDown(GLFW_KEY_A)) movement.x -= 1.0f;
        if (window->isKeyDown(GLFW_KEY_D)) movement.x += 1.0f;
        if (window->isKeyDown(GLFW_KEY_SPACE)) movement.y += 1.0f;
        if (window->isKeyDown(GLFW_KEY_LEFT_SHIFT)) movement.y -= 1.0f;
        if (glm::length(movement) > 0.0f)
        {
            movement = glm::normalize(movement) * cam->getSpeed() * deltaTime;
            cam->move(movement);
        }

        cam->update();

        m_Renderer.draw(mesh, texture, shader, INSTANCE_COUNT);
    }
}

Mesh loadBlockMesh()
{
    std::vector<float> vertices = {
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  // Bottom-left
         0.5f, -0.5f,  0.5f,  1.0f/3.0f, 0.0f,  // Bottom-right
         0.5f,  0.5f,  0.5f,  1.0f/3.0f, 0.5f,  // Top-right
        -0.5f,  0.5f,  0.5f,  0.0f, 0.5f,  // Top-left

        // Back face
        -0.5f, -0.5f, -0.5f,  1.0f/3.0f, 0.0f,  // Bottom-left
         0.5f, -0.5f, -0.5f,  2.0f/3.0f, 0.0f,  // Bottom-right
         0.5f,  0.5f, -0.5f,  2.0f/3.0f, 0.5f,  // Top-right
        -0.5f,  0.5f, -0.5f,  1.0f/3.0f, 0.5f,  // Top-left

        // Left face
        -0.5f, -0.5f, -0.5f,  0.0f, 0.5f,  // Bottom-left
        -0.5f, -0.5f,  0.5f,  1.0f/3.0f, 0.5f,  // Bottom-right
        -0.5f,  0.5f,  0.5f,  1.0f/3.0f, 1.0f,  // Top-right
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  // Top-left

        // Right face
         0.5f, -0.5f, -0.5f,  1.0f/3.0f, 0.5f,  // Bottom-left
         0.5f, -0.5f,  0.5f,  2.0f/3.0f, 0.5f,  // Bottom-right
         0.5f,  0.5f,  0.5f,  2.0f/3.0f, 1.0f,  // Top-right
         0.5f,  0.5f, -0.5f,  1.0f/3.0f, 1.0f,  // Top-left

        // Top face
        -0.5f,  0.5f, -0.5f,  2.0f/3.0f, 0.0f,  // Bottom-left
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f,  // Bottom-right
         0.5f,  0.5f,  0.5f,  1.0f, 0.5f,  // Top-right
        -0.5f,  0.5f,  0.5f,  2.0f/3.0f, 0.5f,  // Top-left

        // Bottom face
        -0.5f, -0.5f, -0.5f,  2.0f/3.0f, 0.5f,  // Bottom-left
         0.5f, -0.5f, -0.5f,  1.0f, 0.5f,  // Bottom-right
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f,  // Top-right
        -0.5f, -0.5f,  0.5f,  2.0f/3.0f, 1.0f   // Top-left
    };

    std::vector<GLsizei> indices = {
        0, 1, 2, 2, 3, 0,       // Front face
        4, 5, 6, 6, 7, 4,       // Back face
        8, 9, 10, 10, 11, 8,    // Left face
        12, 13, 14, 14, 15, 12, // Right face
        16, 17, 18, 18, 19, 16, // Top face
        20, 21, 22, 22, 23, 20  // Bottom face
    };

    const std::shared_ptr<VertexBuffer> vBuffer = std::make_unique<VertexBuffer>(sizeof(float) * 5 * vertices.size(), vertices.data());
    const std::shared_ptr<IndexBuffer> iBuffer = std::make_unique<IndexBuffer>(indices.data(), indices.size());

    VertexBufferLayout vertLayout;
    vertLayout.push<float>(3);
    vertLayout.push<float>(2);

    const unsigned cols = glm::sqrt(INSTANCE_COUNT);
    glm::vec3 translations[INSTANCE_COUNT];

    for (int i = 0; i < cols; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            translations[i * cols + j] = glm::vec3(i, 0, j);
        }
    }

    const std::shared_ptr<VertexBuffer> instanceBuffer = std::make_unique<VertexBuffer>(sizeof(float) * 3 * INSTANCE_COUNT, translations);
    VertexBufferLayout instanceLayout;
    instanceLayout.push<float>(3, false, 1);

    const std::shared_ptr<VertexArray> vArray = std::make_unique<VertexArray>();
    vArray->addBuffer(vBuffer, vertLayout);
    vArray->addBuffer(instanceBuffer, instanceLayout);

    return Mesh(vArray, iBuffer);
}