#ifndef LIST_HPP
#define LIST_HPP

#include <algorithm>
#include <functional>
#include <list>
#include <mutex>

template <typename T>
class List
{
public:
    void for_each(std::function<void(T)> f)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        std::for_each(list_.begin(), list_.end(), f);
    }

    void push(const T& item)
    {
        std::unique_lock<std::mutex> mlock(mutex_);
        list_.push_back(item);
    }

    List() = default;
    List(const List&) = delete;            // disable copying
    List& operator=(const List&) = delete; // disable assignment

private:
    std::list<T> list_;
    std::mutex mutex_;
};

#endif
