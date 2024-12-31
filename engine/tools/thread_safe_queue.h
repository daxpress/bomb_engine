#pragma once
#include <mutex>
#include <queue>

#include "../macros.h"

namespace BE_NAMESPACE
{

template <typename T>
class ThreadSafeQueue
{
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable condition;

public:
    void push(T item)
    {
        {
            std::lock_guard lock(mutex);
            queue.push(std::move(item));
        }
        condition.notify_one();
    }

    void wait_and_pop(T& item)
    {
        std::unique_lock lock(mutex);
        condition.wait(lock, [&] { return !queue.empty(); });
        item = std::move(queue.front());
        queue.pop();
    }
};

}  // namespace BE_NAMESPACE