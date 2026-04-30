#define __LOOP_QUEUE__
#include <atomic>
#include <optional>
#include <vector>

template<typename T>
class LoopQueue {
public:
    LoopQueue(size_t capacity);
    bool push(const T& item);
    bool push(T&& item);
    std::optional<T> pop();

private:
    std::vector<T> queue_;
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
    size_t capacity_;
};

#include "loop_queue.tpp"