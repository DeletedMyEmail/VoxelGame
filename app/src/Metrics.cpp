#include "../include/Metrics.h"
#include <algorithm>
#include <numeric>

void Metrics::update(const double dt)
{
    frameTimes.push_back(dt);
    frameTimeAccumulator += dt;
    while (frameTimeAccumulator > frameTimeWindow)
    {
        frameTimeAccumulator -= frameTimes.front();
        frameTimes.pop_front();
    }
}

double Metrics::getAvgFrameTime() const
{
    const double sum = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0);
    return sum / frameTimes.size();
}

double Metrics::get1PercentLowFrameTime() const
{
    return *std::ranges::max_element(frameTimes);
}