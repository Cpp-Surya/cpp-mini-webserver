#include "MiniWebServer/thread_pool.h"

ThreadPool::ThreadPool(size_t num_threads)
{
    for (size_t i = 0; i < num_threads; ++i)
    {
        workers_.emplace_back(&ThreadPool::workerThread, this);
    }
}

void ThreadPool::enqueue(std::function<void()> task)
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        tasks_.push(std::move(task));
    }
    cv_.notify_one();
}

void ThreadPool::workerThread()
{
    while (true)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            cv_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
            if (stop_ && tasks_.empty())
                return;
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task();
    }
}

void ThreadPool::shutdown()
{
    stop_ = true;
    cv_.notify_all();
    for (auto& t : workers_)
    {
        if (t.joinable())
            t.join();
    }
}

ThreadPool::~ThreadPool() { shutdown(); }
