#pragma once
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

template<typename T>
SafeQueue<T>::SafeQueue(SafeQueue<T>&& other) noexcept{
    std::unique_lock lock(other._mutex);  // Lock the other queue for writing
    _queue = std::move(other._queue);
}

template<typename T>
SafeQueue<T>& SafeQueue<T>::operator=(SafeQueue<T>&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    std::unique_lock lock1(_mutex, std::defer_lock);  // Lock
    std::unique_lock lock2(other._mutex, std::defer_lock);  // Lock the other queue for writing
    std::lock(lock1, lock2);  // Lock both queues without deadlock
    _queue = std::move(other._queue);
    return *this;
}

template<typename T>
void SafeQueue<T>::push(const T& item) {
    std::unique_lock lock(_mutex);
    _queue.push(item);
}

template<typename T>
T SafeQueue<T>::front() const {
    std::shared_lock lock(_mutex);
    return _queue.front();
}

template<typename T>
void SafeQueue<T>::pop() {
    std::unique_lock lock(_mutex);
    _queue.pop();
}

template<typename T>
bool SafeQueue<T>::empty() const {
    std::shared_lock lock(_mutex);
    return _queue.empty();
}

template<typename T>
size_t SafeQueue<T>::size() const {
    std::shared_lock lock(_mutex);
    return _queue.size();
}