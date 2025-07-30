#pragma once
#include <algorithm>
#include <deque>
#include <numeric>
#include <GLFW/glfw3.h>

struct Metrics
{
    float lastTime = 0, frameTimeAccumulator = 0, frameTimeWindow = 5.0f, deltaTime = 0;
    std::deque<float> frameTimes;

    Metrics() : lastTime(glfwGetTime()) {}

    void update();
    float getAvgFrameTime() const;
    float get1PercentLowFrameTime() const;
};
