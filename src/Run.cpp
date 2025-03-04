#include <Player.h>
#include <Renderer.h>
#include "Log.h"
#include "Noise.h"

#define CHUNK_COUNT 4

void createAxesVAO(VertexArray& vao);
void createHighlightVAO(VertexArray& vao);
void moverPlayer(const Window& window, const Camera& cam, Player& player, std::vector<Chunk>& chunks, float deltaTime, bool debugMode);
bool castRay(std::vector<Chunk>& chunks, const glm::vec3& origin, const glm::vec3& direction, float maxDistance, glm::vec3& hitPos);
Chunk& getChunk(glm::vec2 pos, std::vector<Chunk>& m_Chunks);

void run()
{
    Window window;
    Camera cam(glm::vec3{1,Chunk::CHUNK_SIZE+3,1}, 90.0f, window.getWidth(), window.getHeight(), 0.1f, 1000.0f);
    Player player(glm::vec3{1,Chunk::CHUNK_SIZE+3,1});
    Renderer renderer(window.getWidth(), window.getHeight());
    VertexArray highlightVAO, axesVAO;
    createHighlightVAO(highlightVAO);
    createAxesVAO(axesVAO);
    bool debugMode = false;

    // creating chunks
    std::vector<Chunk> chunks;
    constexpr unsigned int size = Chunk::CHUNK_SIZE * CHUNK_COUNT;
    unsigned char** heightMap = genPerlinMap(size, size, Chunk::MAX_HEIGHT / 2, Chunk::MAX_HEIGHT, 42);

    chunks.reserve(CHUNK_COUNT * CHUNK_COUNT);
    for (unsigned int x = 0; x < CHUNK_COUNT; x++)
    {
        for (unsigned int z = 0; z < CHUNK_COUNT; z++)
        {
            chunks.emplace_back(glm::uvec2{x, z}, heightMap);
        }
    }
    freeMap(heightMap, size);

    // set window callbacks
    window.setCursorDisabled(true);
    window.onScroll([&player](Window* win, const glm::dvec2 offset)
         {
             player.increaseSpeed(offset.y);
         });
    window.onKey([&debugMode](Window* win, const int key, const int scancode, const int action, const int mods)
         {
            if (action != GLFW_PRESS)
                return;
            if (key == GLFW_KEY_ESCAPE)
                exit(0);
            if (key == GLFW_KEY_TAB)
                debugMode = !debugMode;
         });
    glm::dvec2 prevCursorPos = window.getMousePosition();
    window.onCursorMove([&cam, &prevCursorPos](Window* win, const glm::dvec2 pos)
        {
            const glm::dvec2 offset = pos - prevCursorPos;
            cam.rotate({offset.x, offset.y});
            prevCursorPos = pos;
        });

    // main loop
    unsigned frameCount = 0;
    float timeSinceDisplay = 0.0f;
    float lastTime = glfwGetTime();
    std::string text;
    while (window.isRunning())
    {
        // calc delta time and display fps
        const float currentTime = glfwGetTime();
        const float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        timeSinceDisplay += deltaTime;
        frameCount++;

        // movement
        moverPlayer(window, cam, player, chunks, deltaTime, debugMode);
        const glm::vec3 camToPlayer = player.getPhysics().getPosition() + glm::vec3(0.5f) - cam.getPosition();
        cam.translate(camToPlayer);
        cam.updateView();

        // highlight block
        if (window.isMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT))
        {
            glm::vec3 blockPos;
            if (castRay(chunks, cam.getPosition(), cam.getLookDir(), 20.0f, blockPos))
                renderer.draw(highlightVAO, TRIANGLES, blockPos, cam);
        }

        // draw
        for (Chunk& chunk : chunks)
            renderer.drawChunk(chunk, cam);

        if (debugMode)
            renderer.draw(axesVAO, LINES, cam.getPosition() + cam.getLookDir(), cam);

        // draw fps
        if (timeSinceDisplay >= 1.0f)
        {
            text = "FPS: " + std::to_string(frameCount) + "  |  Avg frame time: " + std::to_string(timeSinceDisplay / frameCount * 1000) + "ms";
            frameCount = timeSinceDisplay = 0;
        }
        renderer.draw(text.c_str(), glm::ivec2(0), 2);

        renderer.update(window);
    }
}

// experimental ;-; (euphemism for not working)
bool castRay(std::vector<Chunk>& chunks, const glm::vec3& origin, const glm::vec3& direction, const float maxDistance, glm::vec3& hitPos)
{
    const glm::uvec2 chunkPos = getChunkPos(origin);
    const Chunk& chunk = getChunk(chunkPos, chunks);

    const glm::vec3 steps = glm::sign(direction);
    glm::vec3 voxelCoords = origin - glm::vec3(chunkPos.x * Chunk::CHUNK_SIZE, 0, chunkPos.y * Chunk::CHUNK_SIZE);
    glm::vec3 tMax = glm::vec3(voxelCoords + steps) - origin;
    const glm::vec3 tDelta = glm::abs(1.0f / (direction * tMax));

    while(Chunk::inBounds(voxelCoords))
    {
        if (chunk.getBlock(voxelCoords) != BLOCK_TYPE::AIR)
        {
            hitPos = {static_cast<unsigned>(voxelCoords.x) + chunkPos.x * Chunk::CHUNK_SIZE, static_cast<unsigned>(voxelCoords.y), static_cast<unsigned>(voxelCoords.z) + chunkPos.y * Chunk::CHUNK_SIZE};
            return true;
        }

        if(tMax.x < tMax.y)
        {
            if(tMax.x < tMax.z)
            {
                voxelCoords.x += steps.x;
                tMax.x += tDelta.x;
            }
            else
            {
                voxelCoords.z += steps.z;
                tMax.z += tDelta.z;
            }
        }
        else
        {
            if(tMax.y < tMax.z)
            {
                voxelCoords.y += steps.y;
                tMax.y += tDelta.y;
            }
            else
            {
                voxelCoords.z += steps.z;
                tMax.z += tDelta.z;
            }
        }
    }

    return false;
}

void moverPlayer(const Window& window, const Camera& cam, Player& player, std::vector<Chunk>& chunks, const float deltaTime, const bool debugMode)
{
    // movement
    glm::vec3 input(0.0f);
    input.z += 1.0f * window.isKeyDown(GLFW_KEY_W);
    input.z -= 1.0f * window.isKeyDown(GLFW_KEY_S);
    input.x += 1.0f * window.isKeyDown(GLFW_KEY_A);
    input.x -= 1.0f * window.isKeyDown(GLFW_KEY_D);
    input.y += 1.0f * window.isKeyDown(GLFW_KEY_SPACE);
    input.y -= 1.0f * window.isKeyDown(GLFW_KEY_LEFT_SHIFT);

    const glm::vec3 dir = cam.getLookDir();
    const glm::vec3 right = -glm::normalize(cross(dir, glm::vec3(0.0f, 1.0f, 0.0f)));
    const glm::vec3 forward = glm::normalize(glm::vec3{dir.x, 0.0f, dir.z});
    static glm::vec3 up(0.0f, 1.0f, 0.0f);
    const glm::vec3 vel = forward * input.z + right * input.x + up * input.y;

    PhysicsBody& playerBody = player.getPhysics();
    playerBody.addVelocity(vel * deltaTime * player.getSpeed());

    if (debugMode)
    {
        playerBody.update();
        return;
    }

    // physics
    const auto chunkPos = getChunkPos(playerBody.getPosition());
    const Chunk* relevantChunk = nullptr;

    for (auto& chunk : chunks)
    {
        if (chunk.getPosition() == chunkPos)
        {
            relevantChunk = &chunk;
            break;
        }
    }

    if (relevantChunk == nullptr)
        assert(false);

    const unsigned int relativeX = playerBody.getPosition().x - chunkPos.x * Chunk::CHUNK_SIZE;
    const unsigned int relativeY = playerBody.getPosition().y;
    const unsigned int relativeZ = playerBody.getPosition().z - chunkPos.y * Chunk::CHUNK_SIZE;

    const unsigned int xStart = glm::clamp(relativeX, 1u, Chunk::CHUNK_SIZE-4)-1;
    const unsigned int yStart = glm::clamp(relativeY, 1u, Chunk::MAX_HEIGHT-4)-1;
    const unsigned int zStart = glm::clamp(relativeZ, 1u, Chunk::CHUNK_SIZE-4)-1;

    for (unsigned int x = xStart; x < xStart+4; x++)
    {
        for (unsigned int y = yStart; y < yStart+4; y++)
        {
            for (unsigned int z = zStart; z < zStart+4; z++)
            {
                if (relevantChunk->getBlock({x,y,z}) == BLOCK_TYPE::AIR)
                    continue;

                const glm::vec3 pos = glm::vec3(x,y,z) + glm::vec3(chunkPos.x, 0, chunkPos.y) * static_cast<float>(Chunk::CHUNK_SIZE);
                const PhysicsBody blockBounding(pos, {1,1,1}, true);
                playerBody.solveCollision(blockBounding);
            }
        }
    }

    playerBody.update();
}

Chunk& getChunk(const glm::vec2 pos, std::vector<Chunk>& m_Chunks)
{
    const glm::uvec2 chunkPos = {static_cast<unsigned int>(pos.x) / Chunk::CHUNK_SIZE, static_cast<unsigned int>(pos.y) / Chunk::CHUNK_SIZE};
    for (auto& chunk : m_Chunks)
    {
        if (chunk.getPosition() == chunkPos)
            return chunk;
    }

    assert(false);
}

void createAxesVAO(VertexArray& vao)
{
    float axisVertices[] =
    {
        // X axis
        0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0,
        0.5f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0,

        // Y axis
        0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0,
        0.0f, 0.5f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0,

        // Z axis
        0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, 1.0,
        0.0f, 0.0f, 0.5f,   0.0f, 0.0f, 1.0f, 1.0
    };

    const auto vBuffer = std::make_shared<VertexBuffer>(6 * 7 * sizeof(float), axisVertices);
    VertexBufferLayout layout;
    layout.push<float>(3);
    layout.push<float>(4);

    vao.addBuffer(vBuffer, layout);
    vao.setVertexCount(6);
}

void createHighlightVAO(VertexArray& vao)
{
    float vertices[] =
    {
                                                                                // Front
        -0.05f, -0.05f,  1.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Bottom-left
         1.05f, -0.05f,  1.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Bottom-right
         1.05f,  1.05f,  1.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Top-right
        -0.05f, -0.05f,  1.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Bottom-left
         1.05f,  1.05f,  1.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Top-right
        -0.05f,  1.05f,  1.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Top-le0.2         // Back
        -0.05f, -0.05f, -0.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Bottom-left
         1.05f, -0.05f, -0.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Bottom-right
         1.05f,  1.05f, -0.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Top-right
        -0.05f, -0.05f, -0.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Bottom-left
         1.05f,  1.05f, -0.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Top-right
        -0.05f,  1.05f, -0.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Top-le0.2         // Left
        -0.05f, -0.05f, -0.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Bottom-left
        -0.05f,  1.05f, -0.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Top-left
        -0.05f,  1.05f,  1.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Top-right
        -0.05f, -0.05f, -0.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Bottom-left
        -0.05f,  1.05f,  1.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Top-right
        -0.05f, -0.05f,  1.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Bottom-rig0.2     // Right
         1.05f, -0.05f, -0.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Bottom-left
         1.05f,  1.05f, -0.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Top-left
         1.05f,  1.05f,  1.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Top-right
         1.05f, -0.05f, -0.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Bottom-left
         1.05f,  1.05f,  1.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Top-right
         1.05f, -0.05f,  1.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Bottom-rig0.2     // Top
        -0.05f,  1.05f, -0.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Bottom-left
         1.05f,  1.05f, -0.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Bottom-right
         1.05f,  1.05f,  1.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Top-right
        -0.05f,  1.05f, -0.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Bottom-left
         1.05f,  1.05f,  1.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Top-right
        -0.05f,  1.05f,  1.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Top-le0.2         // Bottom
        -0.05f, -0.05f, -0.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Bottom-left
         1.05f, -0.05f, -0.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Bottom-right
         1.05f, -0.05f,  1.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Top-right
        -0.05f, -0.05f, -0.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Bottom-left
         1.05f, -0.05f,  1.05f,    1.0f, 1.0f, 1.0f, 0.05,  // Top-right
        -0.05f, -0.05f,  1.05f,    1.0f, 1.0f, 1.0f, 0.05   // Top-left
    };

    const auto vBuffer = std::make_shared<VertexBuffer>(36 * 7 * sizeof(float), vertices);
    VertexBufferLayout layout;
    layout.push<float>(3);
    layout.push<float>(4);

    vao.addBuffer(vBuffer, layout);
    vao.setVertexCount(36);
}