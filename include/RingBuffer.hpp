#pragma once

#include <vector>

template <typename T>
class RingBuffer {
  public:
    std::vector<T> v;
    std::size_t    size = 0;
    std::size_t    maxSize;
    std::size_t    pos = 0;

    explicit RingBuffer(const std::size_t& maxSize_) : maxSize(maxSize_) { v.reserve(maxSize); }

    void add(const T& data) {
        if (v.size() < maxSize) { // if not full add new
            ++size;
            v.push_back(data);
        } else { // if full increment pos and replace old data
            v[pos] = data;
            ++pos;
            if (pos >= maxSize) pos = 0; // wrap
        }
    }

    void reset() {
        pos = 0;
        v.clear();
    }
};