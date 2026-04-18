#pragma once
#define __SAFE_QUEUE_H__
#include <queue>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <optional>

template<typename T>
class SafeQueue {
public:
    explicit SafeQueue();
    explicit SafeQueue(size_t max_size);
    SafeQueue(const SafeQueue<T>&) = delete;
    SafeQueue& operator=(const SafeQueue<T>&) = delete;
    SafeQueue(SafeQueue<T>&&) noexcept;
    SafeQueue& operator=(SafeQueue<T>&&) noexcept;
    void push(const T& item);
    T front() const;
    std::optional<T> try_pop();
    std::vector<T> drain();
    void pop();
    bool empty() const;
    size_t size() const;
private:
    std::queue<T> _queue;
    mutable std::shared_mutex _mutex;
    std::condition_variable _cv_not_full;
    std::condition_variable _cv_has_items;
    bool _epoll;
    size_t _max_size;
};

#include "safe_queue.tpp"