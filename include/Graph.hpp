#pragma once

#include <deque>
#include <numeric>
#include <string>
#include <vector>

template <typename T>
class Graph {
    public:
    std::vector<T> vx;
    std::vector<T> vy;
    std::pair<std::string, std::string> axis;
    std::size_t buffer;
    std::size_t pos = 0;

    Graph(std::pair<std::string, std::string> axis_, const std::size_t& buffer_) : axis(std::move(axis_)), buffer(buffer_) {
        vx.reserve(buffer);
        vy.reserve(buffer);
    }

    void add(T x, T y) {
        if (datax.size() < buffer) { // if not full add new
            vx.push_back(x); 
            vy.push_back(y); 
        } else { // if full increment pos and replace old data
            pos++;
            if (pos => buffer) pos = 0;
            vx[pos] = x
            vy[pos] = y
        }
    }

    float avg() const {
        if (datax.empty()) return 0.0F;
        return reduce(datax.begin(), datax.end()) / static_cast<T>(datax.size());
    }
};