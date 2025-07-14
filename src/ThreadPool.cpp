#include "ThreadPool.h"

ThreadPool::ThreadPool(const uint32_t numThreads)
{
    for (uint32_t i = 0; i < numThreads; ++i)
        threads.emplace_back(&ThreadPool::threadLoop, this);
}

ThreadPool::~ThreadPool() = default;

void ThreadPool::queueJob(const std::function<void()>& job)
{
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        jobs.push(job);
    }
    mutexCondition.notify_one();
}

void ThreadPool::stop()
{
    for (auto& thread : threads)
        thread.request_stop();
}

bool ThreadPool::busy() const
{
    return busyThreads.load() > 0;
}

void ThreadPool::threadLoop(const std::stop_token& st)
{
    while (!st.stop_requested())
    {
        std::function<void()> job;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            mutexCondition.wait(lock, [&] {
                return !jobs.empty() || st.stop_requested();
            });

            if (st.stop_requested())
                return;

            job = jobs.front();
            jobs.pop();
            ++busyThreads;
        }

        job();
        --busyThreads;
    }
}
