#pragma once

#include <string>
#include <GLFW/glfw3.h>

class FrameMetrics
{
public:
    FrameMetrics() : lastTime(glfwGetTime()) {}

    void update()
    {
        const float currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        deltaSum += deltaTime;
        lastTime = currentTime;
        frameCount++;
    }
    operator std::string() const { return "FPS: " + std::to_string(frameCount) + "  |  Avg frame time: " + std::to_string(deltaSum / frameCount); }
    void reset() { frameCount = 0; deltaSum = 0; }

    float getDeltaSum() const { return deltaSum; }
    float getDeltaTime() const { return deltaTime; }
    unsigned int getFrameCount() const { return frameCount; }
private:
    float lastTime = 0;
    float deltaSum = 0;
    float deltaTime = 0;
    unsigned int frameCount = 0;
};