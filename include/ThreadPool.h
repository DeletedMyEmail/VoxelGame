#pragma once
#include <condition_variable>
#include <functional>
#include <queue>

// https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
struct ThreadPool
{
    ThreadPool(uint32_t numThreads = std::thread::hardware_concurrency());
    void queueJob(const std::function<void()>& job);
    void stop();
    bool busy();

    bool shouldTerminate = false;
    std::mutex queueMutex;
    std::condition_variable mutexCondition;
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> jobs;
private:
    void threadLoop();
};
