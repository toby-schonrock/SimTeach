#pragma once

#include <vector>

template <typename T>
class RingBuffer {
    public:
    std::vector<T> v;
    std::size_t buffer;
    std::size_t pos = 0;

    explicit RingBuffer(const std::size_t& buffer_) : buffer(buffer_) {
        v.reserve(buffer);
    }

    void add(T data) {
        if (v.size() < buffer) { // if not full add new
            v.push_back(data); 
        } else { // if full increment pos and replace old data
            v[pos] = data;
            pos++;
            if (pos >= buffer) pos = 0; // wrap
        }
    }
};