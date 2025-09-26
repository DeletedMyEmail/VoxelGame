#pragma once
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>

// https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
struct ThreadPool
{
    explicit ThreadPool(uint32_t numThreads = std::thread::hardware_concurrency());
    ~ThreadPool();
    void queueJob(const std::function<void()>& job);
    void stop();
    bool busy();
    uint32_t getThreadCount() const { return threads.size(); }

    std::vector<std::jthread> threads;
    std::queue<std::function<void()>> jobs;
    std::mutex queueMutex;
    std::condition_variable mutexCondition;
    std::atomic<uint32_t> busyThreads{0};
private:
    void threadLoop(const std::stop_token& st);
};
