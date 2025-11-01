#include <gtest/gtest.h>

#include <atomic>
#include <chrono>

#include "MiniWebServer/thread_pool.h"

TEST(ThreadPoolTest, ExecutesTasksSuccessfully)
{
    ThreadPool pool(4);
    std::atomic<int> counter{0};

    for (int i = 0; i < 10; ++i)
    {
        pool.enqueue(
            [&counter]
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                counter++;
            });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(counter.load(), 10);
}

TEST(ThreadPoolTest, ShutdownStopsWorkersCleanly)
{
    ThreadPool pool(2);
    pool.enqueue([] { std::this_thread::sleep_for(std::chrono::milliseconds(50)); });
    pool.shutdown();  // should not crash or hang
    SUCCEED();
}
