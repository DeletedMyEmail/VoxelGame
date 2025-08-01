#include "Metrics.h"
#include <cstmlib/Log.h>

void Metrics::update()
{
    const float currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    frameTimes.push_back(deltaTime);
    frameTimeAccumulator += deltaTime;
    while (frameTimeAccumulator > frameTimeWindow)
    {
        frameTimeAccumulator -= frameTimes.front();
        frameTimes.pop_front();
    }
}

float Metrics::getAvgFrameTime() const
{
    const float sum = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0f);
    return sum / frameTimes.size();
}

float Metrics::get1PercentLowFrameTime() const
{
    return *std::ranges::max_element(frameTimes.begin(), frameTimes.end());
}