#include "Profiling.h"

#include <cassert>
#include <Log.h>
#include <sstream>
#include <x86intrin.h>
#include <sys/time.h>

static double toTotalPercent(uint64_t part, uint64_t total);
static uint64_t estimateCPUFreq(uint64_t testPeriod = 100);
static std::string throughputToString(double throughput);

namespace pal
{
    TimeStamp* Profiler::s_TimeStamps = nullptr;
    uint32_t Profiler::s_StampCount = 0;
    uint32_t Profiler::s_MaxStampCount = 0;
    uint64_t Profiler::s_StartTime = 0;

    RepetitionResult repetitionTest(const std::function<void()>& func, const uint64_t bytesProcessed, const uint32_t maxTotalReps, const uint32_t maxRepsSinceLastMin)
    {
        RepetitionResult res{-1u,0,0, 0, 0, 0, 0};
        uint32_t repSinceMin = 0;

        while (res.repetitions < maxTotalReps && repSinceMin < maxRepsSinceLastMin)
        {
            const uint64_t start = __rdtsc();
            func();
            const uint64_t end = __rdtsc();
            const uint64_t elapsedTSCs = end - start;

            if (elapsedTSCs > res.maxTSCs)
            {
                res.maxTSCs = elapsedTSCs;
                res.minThroughput = static_cast<double>(bytesProcessed) / tscsToMs(elapsedTSCs) * 1000.0;;
                res.worstRepIndex = res.repetitions;
                repSinceMin++;
            }
            else if (elapsedTSCs < res.minTSCs)
            {
                res.minTSCs = elapsedTSCs;
                res.maxThroughput = static_cast<double>(bytesProcessed) / tscsToMs(elapsedTSCs) * 1000.0;
                repSinceMin = 0;
                res.bestRepIndex = res.repetitions;
            }
            else
            {
                repSinceMin++;
            }

            res.repetitions++;
        }

        return res;
    }

    RepetitionResult::operator std::string() const
    {
        std::stringstream res;
        res << "Min TSCs:\t\t\t" << minTSCs << " / " << tscsToMs(minTSCs) << " ms\n";
        res << "Best throughput:\t" << throughputToString(maxThroughput) << "\n";
        res << "(at rep:)\t\t\t" << bestRepIndex << "\n";
        res << "Max TSCs:\t\t\t" << maxTSCs << " / " << tscsToMs(maxTSCs) << " ms\n";
        res << "Worst throughput:\t" << throughputToString(minThroughput) << "\n";
        res << "(at rep:)\t\t\t" << worstRepIndex << "\n";
        res << "Total repetitions:\t" << repetitions;

        return res.str();
    }

    void Profiler::init(const uint32_t maxTimeStamps)
    {
        assert(s_TimeStamps == nullptr);

        s_MaxStampCount = maxTimeStamps;
        s_TimeStamps = new TimeStamp[maxTimeStamps];
        s_StartTime = __rdtsc();
    }

    void Profiler::end()
    {
        assert(s_StartTime != 0);
        const uint64_t programTime = __rdtsc() - s_StartTime;

        std::stringstream result;
        result << "------------ Profiler results ------------\n";
        result << "Total time: " << programTime << " tscs / " << tscsToMs(programTime) << " ms\n";

        for(uint32_t i = 0; i < s_StampCount; ++i)
        {
            const auto& [tscs, name] = s_TimeStamps[i];
            result << "Time for \"" << name << "\": " << tscs << " tscs / " << tscsToMs(tscs) << " ms / " << toTotalPercent(tscs, programTime) << "% of total\n";
        }

        delete[] s_TimeStamps;
        LOG_INFO(result.str());
    }

    void Profiler::addTimeStamp(const TimeStamp timeStamp)
    {
        assert(s_StampCount < s_MaxStampCount);
        s_TimeStamps[s_StampCount++] = timeStamp;
    }

    Timer::Timer(const char* name)
        : m_Name(name), m_StartTime(__rdtsc())
    {
    }

    Timer::~Timer()
    {
        const uint64_t endTime = __rdtsc();
        Profiler::addTimeStamp({endTime - m_StartTime, m_Name});
    }

    double tscsToMs(const uint64_t tscs)
    {
        static uint64_t CPUFreq = estimateCPUFreq();

        return static_cast<double>(tscs) * 1000.0 / static_cast<double>(CPUFreq);
    }
}

static std::string throughputToString(double throughput)
{
    std::stringstream res;
    uint8_t unit = 0;
    while (throughput > 1024.0 && unit < 4)
    {
        throughput /= 1024.0;
        unit++;
    }

    res << throughput;

    switch (unit)
    {
        case 0:
            res << " b/s";
            break;
        case 1:
            res << " kb/s";
            break;
        case 2:
            res << " mb/s";
            break;
        case 3:
            res << " gb/s";
            break;
        case 4:
            res << " tb/s";
            break;
        default:
            assert(false);
    }

    return res.str();
}

static double toTotalPercent(const uint64_t part, const uint64_t total)
{
    if (total == 0)
        return 0.0;

    return (static_cast<double>(part) / static_cast<double>(total)) * 100.0;
}

static uint64_t readOSTime()
{
    timeval val{};
    gettimeofday(&val, nullptr);

    return 1000000 * static_cast<uint64_t>(val.tv_sec) + static_cast<uint64_t>(val.tv_usec);
}

static uint64_t estimateCPUFreq(const uint64_t testPeriod)
{
    uint64_t freq = 1000000;
    uint64_t CPUStart = __rdtsc();
    uint64_t OSStart = readOSTime();
    uint64_t OSWaitTime = freq * testPeriod / 1000;

    uint64_t OSEnd = 0;
    uint64_t OSElapsed = 0;
    while(OSElapsed < OSWaitTime)
    {
        OSEnd = readOSTime();
        OSElapsed = OSEnd - OSStart;
    }

    uint64_t CPUEnd = __rdtsc();
    uint64_t CPUElapsed = CPUEnd - CPUStart;

    uint64_t CPUFreq = freq * CPUElapsed / OSElapsed;

    return CPUFreq;
}
