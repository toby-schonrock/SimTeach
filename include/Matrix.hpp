#pragma once

#include <vector>

template <typename T>
struct Matrix {
    std::vector<T> v;
    int            sizeX = 0;
    int            sizeY = 0;

    Matrix(int x_, int y_) : v(static_cast<std::size_t>(x_ * y_), T{}), sizeX(x_), sizeY(y_) {}

    T& operator()(int x_, int y_) { return v[static_cast<std::size_t>(x_ + y_ * sizeX)]; }

    const T& operator()(int x_, int y_) const { return v[static_cast<std::size_t>(x_ + y_ * sizeX)]; }
};
