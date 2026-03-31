#pragma once
#include <queue>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>

template<typename T>
class SafeQueue {
public:
    explicit SafeQueue() = default;
    SafeQueue(const SafeQueue&) = delete;
    SafeQueue& operator=(const SafeQueue&) = delete;
    SafeQueue(SafeQueue&&) noexcept;
    SafeQueue& operator=(SafeQueue&&) noexcept;
    void push(const T& item);
    T front() const;
    void pop();
    bool empty() const;
    size_t size() const;
private:
    std::queue<T> _queue;
    mutable std::shared_mutex _mutex;
};