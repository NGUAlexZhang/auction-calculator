#ifndef __SAFE_QUEUE_TPP__
#include "safe_queue.h"
#endif

template<typename T>
SafeQueue<T>::SafeQueue() : _capacity(0), _batch_threshold(1) {}

template<typename T>
SafeQueue<T>::SafeQueue(size_t batch_threshold)
    : _capacity(0), _batch_threshold(std::max<size_t>(1, batch_threshold)) {}

template<typename T>
SafeQueue<T>::SafeQueue(size_t capacity, size_t batch_threshold)
    : _capacity(capacity),
      _batch_threshold(std::max<size_t>(1, batch_threshold)) {}

template<typename T>
SafeQueue<T>::SafeQueue(SafeQueue<T>&& other) noexcept{
    std::unique_lock lock(other._mutex);  // Lock the other queue for writing
    _queue = std::move(other._queue);
    _capacity = other._capacity;
    _batch_threshold = other._batch_threshold;
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
    _capacity = other._capacity;
    _batch_threshold = other._batch_threshold;
    return *this;
}

template<typename T>
void SafeQueue<T>::push(const T& item) {
    std::unique_lock lock(_mutex);
    _cv_not_full.wait(lock, [this]() { return _capacity == 0 || _queue.size() < _capacity; });
    _queue.push(item);
    if (_queue.size() >= _batch_threshold) {
        _cv_has_items.notify_one();
    }
}

template<typename T>
std::optional<T> SafeQueue<T>::try_pop() {
    std::unique_lock lock(_mutex);
    if (_queue.empty()) {
        return std::nullopt;
    }
    T item = _queue.front();
    _queue.pop();
    _cv_not_full.notify_one();
    return item;
}

template<typename T>
std::vector<T> SafeQueue<T>::drain() {
    std::unique_lock lock(_mutex);
    _cv_has_items.wait(lock, [this]() { return _queue.size() >= _batch_threshold; });
    std::vector<T> items;
    while (!_queue.empty()) {
        items.push_back(std::move(_queue.front()));
        _queue.pop();
    }
    _cv_not_full.notify_all();
    return items;
}

template<typename T>
template<typename Rep, typename Period>
std::vector<T> SafeQueue<T>::drain_for(
    const std::chrono::duration<Rep, Period>& timeout) {
    std::unique_lock lock(_mutex);
    _cv_has_items.wait_for(lock, timeout, [this]() {
        return !_queue.empty() && _queue.size() >= _batch_threshold;
    });
    std::vector<T> items;
    while (!_queue.empty()) {
        items.push_back(std::move(_queue.front()));
        _queue.pop();
    }
    _cv_not_full.notify_all();
    return items;
}

template<typename T>
T SafeQueue<T>::front() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _queue.front();
}

template<typename T>
void SafeQueue<T>::pop() {
    std::unique_lock lock(_mutex);
    _queue.pop();
    _cv_not_full.notify_one();
}

template<typename T>
bool SafeQueue<T>::empty() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _queue.empty();
}

template<typename T>
size_t SafeQueue<T>::size() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _queue.size();
}
