#pragma once

#include <cstdint>
#include <vector>

template <typename T>
struct Matrix {
    std::vector<T> v;
    std::uint32_t            sizeX = 0;
    std::uint32_t            sizeY = 0;

    Matrix(std::uint32_t x_, std::uint32_t y_) : v(x_ * y_, T{}), sizeX(x_), sizeY(y_) {}

    T& operator()(int x_, int y_) { return v[x_ + y_ * sizeX]; }

    const T& operator()(int x_, int y_) const { return v[static_cast<std::uint32_t>(x_ + y_ * sizeX)]; }
};
