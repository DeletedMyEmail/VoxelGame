#include "App.h"
#include "Chunk.h"
#include "Noise.h"
#include "Renderer.h"
#include "stb/stb_image_write.h"


void App::run()
{
    Camera cam(glm::vec3{0,0,-2}, 90.0f, 1200, 1900, 0.1f, 1000.0f);
    Window window = WindowBuilder().
            size(1200, 1900).
            title("Minecraft Clone").
            disableCursor().
            onScroll([&cam](Window* win, const double x, const double y)
            {
                cam.increaseSpeed(y);
            }).
            onClose([](Window*)
            {
                exit(0);
            }).
            build();
    window.bind();

    Shader shader("../shader/BlockVert.glsl", "../shader/BlockFrag.glsl");
    BlockRenderer renderer("../resources/textureAtlas.png");

    Chunk chunk(glm::vec2(0));

    auto prevMousePos = window.getMousePosition();
    float lastTime = glfwGetTime();
    float deltaSum = 0;
    unsigned frameCount = 0;

    while (window.isRunning()) {
        const float currentTime = glfwGetTime();
        const float deltaTime = currentTime - lastTime;
        deltaSum += deltaTime;
        lastTime = currentTime;
        frameCount++;

        if (deltaSum >= 1.0)
        {
            window.setTitle("Minecraft Clone  |  FPS: " + std::to_string(frameCount) + "  |  Avg frame time: " + std::to_string(deltaSum / frameCount));
            deltaSum = 0;
            frameCount = 0;
        }

        renderer.clear(window);

        if (window.isKeyDown(GLFW_KEY_ESCAPE)) exit(0);

        const auto mousePos = window.getMousePosition();
        if (mousePos != prevMousePos)
        {
            const auto relMouseMovement = mousePos - prevMousePos;

            cam.rotate(relMouseMovement.x, relMouseMovement.y);
            prevMousePos = mousePos;
        }

        glm::vec3 movement(0.0f);
        if (window.isKeyDown(GLFW_KEY_W)) movement.z += 1.0f;
        if (window.isKeyDown(GLFW_KEY_S)) movement.z -= 1.0f;
        if (window.isKeyDown(GLFW_KEY_A)) movement.x -= 1.0f;
        if (window.isKeyDown(GLFW_KEY_D)) movement.x += 1.0f;
        if (window.isKeyDown(GLFW_KEY_SPACE)) movement.y += 1.0f;
        if (window.isKeyDown(GLFW_KEY_LEFT_SHIFT)) movement.y -= 1.0f;
        if (glm::length(movement) > 0.0f)
        {
            movement = glm::normalize(movement) * cam.getSpeed() * deltaTime;
            cam.move(movement);
        }

        cam.update();
        renderer.draw(chunk, shader, window, cam);
    }
}
