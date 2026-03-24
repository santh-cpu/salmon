#pragma once
#include <algorithm>
#include <array>
#include <atomic>

template <typename T, size_t Cap> class RingBuffer {
  static_assert((Cap & (Cap - 1)) == 0, "Cap must be power of 2");
  static constexpr size_t MASK = Cap - 1;
  alignas(64) std::atomic<size_t> head_{0};
  alignas(64) std::atomic<size_t> tail_{0};
  alignas(64) std::array<T, Cap> buf_;

public:
  bool push(const T *data, size_t n);
  size_t pop(T *data, size_t n);
  size_t available() const;
};

template <typename T, size_t Cap>
bool RingBuffer<T, Cap>::push(const T *data, size_t n) {
  size_t head = head_.load(std::memory_order_relaxed);
  size_t tail = tail_.load(std::memory_order_acquire);

  size_t freeSpace = Cap - (head - tail);
  if (n > freeSpace)
    return false;

  for (size_t i = 0; i < n; ++i) {
    buf_[(head + i) & MASK] = data[i];
  }

  head_.store(head + n, std::memory_order_release);
  return true;
}

template <typename T, size_t Cap>
size_t RingBuffer<T, Cap>::pop(T *data, size_t n) {
  size_t head = head_.load(std::memory_order_acquire);
  size_t tail = tail_.load(std::memory_order_relaxed);

  size_t available = head - tail;
  size_t toRead = std::min(n, available);

  for (size_t i = 0; i < toRead; ++i) {
    data[i] = buf_[(tail + i) & MASK];
  }

  tail_.store(tail + toRead, std::memory_order_release);
  return toRead;
}

template <typename T, size_t Cap> size_t RingBuffer<T, Cap>::available() const {
  size_t head = head_.load(std::memory_order_acquire);
  size_t tail = tail_.load(std::memory_order_acquire);
  return head - tail;
}
