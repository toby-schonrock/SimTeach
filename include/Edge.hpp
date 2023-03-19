#pragma once

#include "Vector2.hpp"

class Edge {
  private:
    Vec2   p1_;
    Vec2   p2_;
    Vec2   min_;
    Vec2   max_;
    Vec2   diff_;
    double mag_;
    Vec2   unitDiff_;
    Vec2   normal_;

  public:
    [[nodiscard]] constexpr const Vec2& p1() const { return p1_; }
    [[nodiscard]] constexpr const Vec2& p2() const { return p2_; }
    [[nodiscard]] constexpr const Vec2& min() const { return min_; }
    [[nodiscard]] constexpr const Vec2& max() const { return max_; }
    [[nodiscard]] constexpr const Vec2& diff() const { return diff_; }
    [[nodiscard]] constexpr double      mag() const { return mag_; }
    [[nodiscard]] constexpr const Vec2& unitDiff() const { return unitDiff_; }
    [[nodiscard]] constexpr const Vec2& normal() const { return normal_; }

    Edge(const Vec2& p1, const Vec2& p2)
        : p1_(p1), p2_(p2), min_(std::min(p1.x, p2.x), std::min(p1.y, p2.y)),
          max_(std::max(p1.x, p2.x), std::max(p1.y, p2.y)), diff_(p2 - p1), mag_(diff_.mag()),
          unitDiff_(diff_ / mag_), normal_(-unitDiff_.y, unitDiff_.x) {}

    void set(const Vec2& p1, const Vec2& p2) {
        p1_       = p1;
        p2_       = p2;
        min_.x    = std::min(p1.x, p2.x);
        min_.y    = std::min(p1.y, p2.y);
        max_.x    = std::max(p1.x, p2.x);
        max_.y    = std::max(p1.y, p2.y);
        diff_     = p2 - p1;
        mag_      = diff_.mag();
        unitDiff_ = diff_ / mag_;
        normal_   = {-unitDiff_.y, unitDiff_.x};
    }

    void p1(const Vec2& p1) { set(p1, p2_); }

    void p2(const Vec2& p2) { set(p1_, p2); }

    double distToPoint(const Vec2& pos) const {
        return std::abs(unitDiff_.cross(p1_ - pos));
    }

    // checks if ray cast from point upwards collides with edge
    bool rayCast(const Vec2& pos) const {
        if (diff_.x == 0)
            return false; // if vertices form a verticle line a verticle line cannot intersect
        if (pos.x == p1_.x && pos.y < p1_.y)
            return !std::signbit(diff_.x); // perfect vertical allignment with one end
        if (pos.x == p2_.x && pos.y < p2_.y)
            return std::signbit(diff_.x); // perfect vertical allignment with one end
        if (pos.x < min_.x || pos.x > max_.x) return false; // outside x range
        return (pos.x - p1_.x) / diff_.x * diff_.y + p1_.y > pos.y;
    }
};