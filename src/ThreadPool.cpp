#include "ThreadPool.h"

ThreadPool::ThreadPool(const uint32_t numThreads)
{
    for (uint32_t i = 0; i < numThreads; ++i)
        threads.emplace_back(std::thread(&ThreadPool::threadLoop,this));
}

void ThreadPool::queueJob(const std::function<void()>& job)
{
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        jobs.push(job);
    }
    mutexCondition.notify_one();
}

void ThreadPool::stop()
{
    shouldTerminate = true;
    mutexCondition.notify_all();
}

bool ThreadPool::busy()
{
    bool isBusy;
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        isBusy = !jobs.empty();
    }

    return isBusy;
}

void ThreadPool::threadLoop()
{
    while (true)
    {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            mutexCondition.wait(lock, [this] {
                return !jobs.empty() || shouldTerminate;
            });
            if (shouldTerminate)
                return;

            job = jobs.front();
            jobs.pop();
        }

        job();
    }
}
