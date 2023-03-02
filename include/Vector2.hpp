#pragma once

#include "Util.hpp"

#include <cmath>
#include <iostream>
#include <ostream>

template <typename T>
class Vector2 {
  public:
    T x{};
    T y{};

    constexpr Vector2() = default;

    constexpr Vector2(T x_, T y_) : x(x_), y(y_) {}

    template <typename U>
    explicit operator Vector2<U>() const {
        return Vector2<U>(static_cast<U>(x), static_cast<U>(y));
    }

    constexpr T       mag() const { return std::hypot(x, y); }
    constexpr Vector2 norm() const { return *this / this->mag(); }
    constexpr Vector2 abs() const { return {std::abs(x), std::abs(y)}; }
    constexpr double  dot(const Vector2& rhs) const { return x * rhs.x + y * rhs.y; }
    constexpr double  cross(const Vector2& rhs) const { return x * rhs.y - y * rhs.x; }
    constexpr T       distToLine(const Vector2& p1, const Vector2& p2) const {
        Vector2 line  = p2 - p1;
        Vector2 diff1 = *this - p1;
        if (diff1.dot(line) < 0) return diff1.mag();
        Vector2 diff2 = *this - p2;
        if (diff2.dot(line) > 0) return diff2.mag();
        return std::abs(line.cross(diff1)) / line.mag();
    }

    // clang-format off
    constexpr Vector2& operator+=(const Vector2& obj) { x += obj.x; y += obj.y; return *this; }
    constexpr Vector2& operator-=(const Vector2& obj) { x -= obj.x; y -= obj.y; return *this; }
    constexpr Vector2& operator*=(double scale) { x *= scale; y *= scale; return *this; }
    constexpr Vector2& operator/=(double scale) { x /= scale; y /= scale; return *this; }
    // clang-format on

    constexpr friend Vector2 operator+(Vector2 lhs, const Vector2& rhs) { return lhs += rhs; }
    constexpr friend Vector2 operator-(Vector2 lhs, const Vector2& rhs) { return lhs -= rhs; }
    constexpr friend Vector2 operator*(Vector2 lhs, double scale) { return lhs *= scale; }
    constexpr friend Vector2 operator*(double scale, Vector2 rhs) { return rhs *= scale; }
    constexpr friend Vector2 operator/(Vector2 lhs, double scale) { return lhs /= scale; }
    constexpr friend bool operator==(const Vector2&  lhs, const Vector2& rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
    constexpr friend bool operator!=(const Vector2&  lhs, const Vector2& rhs) { return lhs.x != rhs.x || lhs.y != rhs.y; }


    friend std::ostream& operator<<(std::ostream& os, const Vector2& v) {
        return os << '(' << v.x << ", " << v.y << ')';
    }

    friend std::istream& operator>>(std::istream& is, Vector2& v) {
        safeStreamRead(is, v.x);
        safeStreamRead(is, v.y);
        return is;
    }
};

using Vec2  = Vector2<double>;
using Vec2I = Vector2<int>;
using Vec2U = Vector2<unsigned>;
using Vec2F = Vector2<float>;
