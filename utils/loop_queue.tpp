#ifndef __LOOP_QUEUE__
#include "loop_queue.hpp"
#endif
#include <iostream>

template<typename T>
LoopQueue<T>::LoopQueue(size_t capacity) {
    if(capacity == 0 || ((capacity & (capacity - 1)) == 0)) {
        // error capacity must be 2^n
    }
    queue_.reserve(capacity);
    head_ = 0;
    tail_ = 0;
    capacity_ = capacity;
}

template<typename T>
bool LoopQueue<T>::push(const T& item) {
    auto head = head_.load(std::memory_order_relaxed);
    auto tail = tail_.load(std::memory_order_relaxed);

    if(tail - head == capacity_) {
        return false;
    }

    queue_[tail & (capacity_ - 1)] = item;

    tail_.store(tail + 1, std::memory_order_release);
    return true;
}

template<typename T>
bool LoopQueue<T>::push(T&& item) {
    auto head = head_.load(std::memory_order_relaxed);
    auto tail = tail_.load(std::memory_order_relaxed);

    if(tail - head == capacity_) {
        return false;
    }

    queue_[tail & (capacity_ - 1)] = std::move(item);

    tail_.store(tail + 1, std::memory_order_release);
    return true;
}

template<typename T>
std::optional<T> LoopQueue<T>::pop() {
    auto head = head_.load(std::memory_order_relaxed);
    auto tail = tail_.load(std::memory_order_relaxed);

    if(tail == head) {
        return std::nullopt;
    }

    auto item = queue_[head & (capacity_ - 1)];
    head_.store(head + 1, std::memory_order_release);
    return item;
}