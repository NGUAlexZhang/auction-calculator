#pragma once
#define __SAFE_QUEUE_H__
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

template<typename T>
class SafeQueue {
public:
    explicit SafeQueue();
    explicit SafeQueue(size_t batch_threshold);
    SafeQueue(size_t capacity, size_t batch_threshold);
    SafeQueue(const SafeQueue<T>&) = delete;
    SafeQueue& operator=(const SafeQueue<T>&) = delete;
    SafeQueue(SafeQueue<T>&&) noexcept;
    SafeQueue& operator=(SafeQueue<T>&&) noexcept;
    void push(const T& item);
    T front() const;
    std::optional<T> try_pop();
    std::vector<T> drain();
    template<typename Rep, typename Period>
    std::vector<T> drain_for(const std::chrono::duration<Rep, Period>& timeout);
    void pop();
    bool empty() const;
    size_t size() const;
private:
    std::queue<T> _queue;
    mutable std::mutex _mutex;
    std::condition_variable _cv_not_full;
    std::condition_variable _cv_has_items;
    size_t _capacity;
    size_t _batch_threshold;
};

#include "safe_queue.tpp"
