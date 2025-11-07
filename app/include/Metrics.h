#pragma once
#include "RingBuffer.h"
#include <string>
#include <unordered_map>
#include "Application.h"
#include "imgui.h"

const size_t SAMPLE_COUNT = 500;

struct MetricData
{
    MetricData() : values()
    {
        color.x = static_cast<float>(rand() % 256) / 255.0f;
        color.y = static_cast<float>(rand() % 256) / 255.0f;
        color.z = static_cast<float>(rand() % 256) / 255.0f;
        color.w = 1.0f;
    }

    RingBuffer<double, SAMPLE_COUNT> values;
    ImVec4 color;
};

#ifdef NOPROFILE
    #define CAPTURE(name, func) func;
#else
    inline std::unordered_map<std::string, MetricData> metrics;

    #define CAPTURE(name, func) { \
        double start = core::Application::get().getTime(); \
        func; \
        double end = core::Application::get().getTime(); \
        double ms = (end - start) * 1000.0; \
        metrics[name].values.push(ms); \
    }
#endif
