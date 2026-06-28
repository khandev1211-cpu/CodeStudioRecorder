#pragma once
#include <atomic>
#include <vector>
#include <optional>

namespace cs {

template<typename T, size_t Capacity>
class SPSCQueue {
public:
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");

    bool push(const T& item) {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t next_head = (head + 1) & mask_;

        if (next_head == tail_.load(std::memory_order_acquire)) {
            return false; // Queue full
        }

        buffer_[head] = item;
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        size_t tail = tail_.load(std::memory_order_relaxed);

        if (tail == head_.load(std::memory_order_acquire)) {
            return false; // Queue empty
        }

        item = buffer_[tail];
        tail_.store((tail + 1) & mask_, std::memory_order_release);
        return true;
    }

    size_t size() const {
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);
        return (head - tail) & mask_;
    }

private:
    T buffer_[Capacity];
    const size_t mask_ = Capacity - 1;
    std::atomic<size_t> head_{ 0 };
    std::atomic<size_t> tail_{ 0 };
};

} // namespace cs
