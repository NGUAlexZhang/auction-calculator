#pragma once
#define __SAFE_QUEUE_H__
#include <queue>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>

template<typename T>
class SafeQueue {
public:
    explicit SafeQueue() = default;
    SafeQueue(const SafeQueue<T>&) = delete;
    SafeQueue& operator=(const SafeQueue<T>&) = delete;
    SafeQueue(SafeQueue<T>&&) noexcept;
    SafeQueue& operator=(SafeQueue<T>&&) noexcept;
    void push(const T& item);
    T front() const;
    void pop();
    bool empty() const;
    size_t size() const;
private:
    std::queue<T> _queue;
    mutable std::shared_mutex _mutex;
};

#include "safe_queue.tpp"