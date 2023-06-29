#pragma once

#include "Fundamentals/Vector2.hpp"
#include "SFML/Graphics.hpp"
#include <algorithm>
#include <cstddef>
#include <string>

sf::Vector2f visualize(const Vec2& v);

struct Point {
  public:
    sf::Color color;
    Vec2      pos;
    Vec2      vel{};
    Vec2      f;
    double    mass = 1.0;
    bool      fixed;

    Point() = default;

    Point(const Vec2& pos_, double mass_, const sf::Color& color_, bool fixed_)
        : color(color_), pos(pos_), mass(mass_), fixed(fixed_) {}

    void update(double deltaTime, double gravity) {
        if (!fixed) {
            vel += (f / mass + Vec2(0, -gravity)) *
                   deltaTime; // TODO euler integration could be improved (e.g. runge kutta)
            pos += vel * deltaTime;
        }
        f = Vec2();
    }

    // used for saving point to file as text
    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
        return os << p.fixed << ' ' << p.pos.x << ' ' << p.pos.y << ' ' << p.vel.x << ' ' << p.vel.y
                  << ' ' << p.mass << ' ' << std::to_string(p.color.r) << ' '
                  << std::to_string(p.color.g) << ' ' << std::to_string(p.color.b) << ' '
                  << std::to_string(p.color.a);
    }

    // used for creating point from file
    friend std::istream& operator>>(std::istream& is, Point& p) {
        safeStreamRead(is, p.fixed);
        safeStreamRead(is, p.pos);
        safeStreamRead(is, p.vel);
        safeStreamRead(is, p.mass);
        safeStreamRead(is, p.color.r);
        safeStreamRead(is, p.color.g);
        safeStreamRead(is, p.color.b);
        safeStreamRead(is, p.color.a);
        if (is.good()) {
            throw std::runtime_error("To many columns for a point - file invalid");
        }
        return is;
    }
};

template <typename T>
struct Index {
    std::size_t id    = 0;
    constexpr Index() = default;
    explicit Index(std::size_t id_) : id(std::move(id_)) {}
    explicit         operator std::size_t() const { return id; }
    explicit         operator long long() const { return static_cast<long long>(id); }
    auto             operator<=>(const Index& obj) const = default;
    constexpr Index& operator+=(const Index& obj) {
        id += obj.id;
        return *this;
    }
    constexpr Index& operator-=(const Index& obj) {
        id -= obj.id;
        return *this;
    }
    constexpr friend Index operator+(Index lhs, const Index& rhs) { return lhs += rhs; }
    constexpr friend Index operator-(Index lhs, const Index& rhs) { return lhs -= rhs; }
    constexpr Index&       operator++() {
              ++id;
              return *this;
    }
    constexpr Index& operator--() {
        --id;
        return *this;
    }
    friend std::ostream& operator<<(std::ostream& os, const Index& i) {
        return os << std::to_string(i.id);
    }
    friend std::istream& operator>>(std::istream& is, Index& i) {
        safeStreamRead(is, i.id);
        return is;
    }
};

using PointId = Index<Point>;