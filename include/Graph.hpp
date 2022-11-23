#pragma once

#include <deque>
#include <numeric>
#include <string>
#include <vector>

template <typename T>
class Graph {
    private:
    std::vector<T> vx;
    std::vector<T> vy;

    public:
    std::deque<T> datax;
    std::deque<T> datay;
    std::pair<std::string, std::string> axis;
    std::size_t buffer;

    Graph(std::pair<std::string, std::string> axis_, const std::size_t& buffer_) : axis(std::move(axis_)), buffer(buffer_) {
        vx.reserve(buffer);
        vy.reserve(buffer);
    }

    void add(T x, T y) {
        if (datax.size() == buffer) { // if full remove first item
            datax.pop_front(); 
            datay.pop_front(); 
        }
        datax.push_back(x);
        datay.push_back(y);
    }

    float avg() const {
        if (datax.empty()) return 0.0F;
        return reduce(datax.begin(), datax.end()) / static_cast<T>(datax.size());
    }

    std::pair<T*, T*> arr(){
        vx = {datax.begin(), datax.end()};
        vy = {datax.begin(), datax.end()};
        return {vx.data(), vy.data()};
    }
};