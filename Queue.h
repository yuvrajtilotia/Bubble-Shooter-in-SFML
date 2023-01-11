#pragma once
#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class Queue
{
public:
    T pop() // Blocking pop
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        cond_.wait(mlock, [this] {return !queue_.empty(); });
        auto val = queue_.front();
        queue_.pop();
        return val;
    }

    void pop(T& item) // pop will not wait with an empty queue
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        if (queue_.empty())
        {
            return;
        }
        item = queue_.front();
        queue_.pop();
    }

    void push(const T& item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        queue_.push(item);
        cond_.notify_one();
    }
    Queue() = default;
    Queue(const Queue&) = delete;            // disable copying
    Queue& operator=(const Queue&) = delete; // disable assignment

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

#endif