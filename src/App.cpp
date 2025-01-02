#include "App.h"
#include "Chunk.h"
#include "Noise.h"
#include "Renderer.h"
#include "Metrics.h"

void createChunks(unsigned int chunksPerSide, std::vector<Chunk>& chunks);
glm::vec3 getCamMovement(const Window& window);
void processCamInputs(Window& window, Camera& cam, glm::dvec2& prevMousePos, float deltaTime);

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
            onKey([](Window* win, const int key, const int scancode, const int action, int mods) {
                if (key == GLFW_KEY_ESCAPE)
                    exit(0);
                if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
                {
                    GLint mode;
                    GLCall(glGetIntegerv(GL_POLYGON_MODE, &mode))
                    GLCall(glPolygonMode( GL_FRONT_AND_BACK, mode == GL_FILL ? GL_LINE : GL_FILL );)
                }
            }).
            build();
    Shader shader("../shader/BlockVert.glsl", "../shader/BlockFrag.glsl", "../shader/BlockGeo.glsl");
    BlockRenderer renderer("../resources/textureAtlas.png");

    std::vector<Chunk> chunks;
    createChunks(4, chunks);

    auto prevMousePos = window.getMousePosition();
    FrameMetrics metrics;
    while (window.isRunning())
    {
        metrics.update();
        if (metrics.getDeltaSum() >= 1.0f)
        {
            window.setTitle(metrics);
            metrics.reset();
        }

        renderer.clear(window);

        processCamInputs(window, cam, prevMousePos, metrics.getDeltaTime());
        cam.update();

        for (auto& chunk : chunks)
            renderer.draw(chunk, shader, window, cam);
    }
}

void processCamInputs(Window& window, Camera& cam, glm::dvec2& prevMousePos, float deltaTime)
{
    const auto mousePos = window.getMousePosition();
    if (mousePos != prevMousePos)
    {
        const auto relMouseMovement = mousePos - prevMousePos;

        cam.rotate(relMouseMovement.x, relMouseMovement.y);
        prevMousePos = mousePos;
    }

    auto movement = getCamMovement(window);
    if (movement != glm::vec3(0))
    {
        movement = glm::normalize(movement) * cam.getSpeed() * deltaTime;
        cam.move(movement);
    }
}

glm::vec3 getCamMovement(const Window& window)
{
    glm::vec3 movement(0.0f);
    if (window.isKeyDown(GLFW_KEY_W)) movement.z += 1.0f;
    if (window.isKeyDown(GLFW_KEY_S)) movement.z -= 1.0f;
    if (window.isKeyDown(GLFW_KEY_A)) movement.x -= 1.0f;
    if (window.isKeyDown(GLFW_KEY_D)) movement.x += 1.0f;
    if (window.isKeyDown(GLFW_KEY_SPACE)) movement.y += 1.0f;
    if (window.isKeyDown(GLFW_KEY_LEFT_SHIFT)) movement.y -= 1.0f;

    return movement;
}

void createChunks(const unsigned int chunksPerSide, std::vector<Chunk>& chunks)
{
    const unsigned int size = Chunk::CHUNK_SIZE * chunksPerSide;
    unsigned char** heightMap = genPerlinMap(size, size, Chunk::MAX_HEIGHT / 2, Chunk::MAX_HEIGHT, std::time(nullptr));

    chunks.reserve(chunksPerSide * chunksPerSide);
    for (unsigned int x = 0; x < chunksPerSide; x++)
    {
        for (unsigned int z = 0; z < chunksPerSide; z++)
        {
            chunks.emplace_back(glm::uvec2{x, z}, heightMap);
        }
    }

    freeMap(heightMap, size);
}
