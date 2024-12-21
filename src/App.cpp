#include "App.h"

Mesh loadBlockMesh();

App::App()
    : m_Renderer(WindowBuilder().
                size(1200, 1900).
                title("Minecraft Clone").
                disableCursor().
                onClose([](Window*) {exit(0);}).
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
    const Texture texture("../resources/block.png");
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

        m_Renderer.draw(mesh, texture, shader);
    }
}

Mesh loadBlockMesh()
{
    std::vector<float> vertices = {
        // Vertex positions followed by texture coordinates
        1.0f,  1.0f, -1.0f, 0.625f, 0.500f, // Vertex 1
        1.0f, -1.0f, -1.0f, 0.375f, 0.500f, // Vertex 2
        1.0f,  1.0f,  1.0f, 0.625f, 0.750f, // Vertex 3
        1.0f, -1.0f,  1.0f, 0.375f, 0.750f, // Vertex 4
       -1.0f,  1.0f, -1.0f, 0.875f, 0.500f, // Vertex 5
       -1.0f, -1.0f, -1.0f, 0.625f, 0.250f, // Vertex 6
       -1.0f,  1.0f,  1.0f, 0.125f, 0.500f, // Vertex 7
       -1.0f, -1.0f,  1.0f, 0.375f, 0.250f  // Vertex 8
    };

    std::vector<GLsizei> indices = {
        4, 2, 0,  // Face 1
        2, 7, 3,  // Face 2
        6, 5, 7,  // Face 3
        1, 7, 5,  // Face 4
        0, 3, 1,  // Face 5
        4, 1, 5,  // Face 6
        4, 6, 2,  // Face 7
        2, 6, 7,  // Face 8
        6, 4, 5,  // Face 9
        1, 3, 7,  // Face 10
        0, 2, 3,  // Face 11
        4, 0, 1   // Face 12
    };

    const std::shared_ptr<VertexBuffer> vBuffer = std::make_unique<VertexBuffer>(sizeof(float) * 5 * vertices.size(), vertices.data());
    const std::shared_ptr<IndexBuffer> iBuffer = std::make_unique<IndexBuffer>(indices.data(), indices.size());

    VertexBufferLayout layout;
    layout.push<float>(3);
    layout.push<float>(2);

    const std::shared_ptr<VertexArray> vArray = std::make_unique<VertexArray>();
    vArray->addBuffer(vBuffer, layout);

    return Mesh(vArray, iBuffer);
}