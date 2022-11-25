#pragma once

#include <cstdint>
#include <vector>

template <typename T>
class RingBuffer {
    public:
    std::vector<T> v;
    std::uint32_t size;
    std::uint32_t pos = 0;

    explicit RingBuffer(const std::uint32_t& size_) : size(size_) {
        v.reserve(size);
    }

    void add(const T& data) {
        if (v.size() < size) { // if not full add new
            v.push_back(data); 
        } else { // if full increment pos and replace old data
            v[pos] = data;
            pos++;
            if (pos >= size) pos = 0; // wrap
        }
    }
};