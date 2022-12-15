#pragma once

#include "Vector2.hpp"

class Edge {
  public:
    Vec2   p1;
    Vec2   p2;
    Vec2   min;
    Vec2   max;
    Vec2   diff;
    double mag;
    Vec2   unitDiff;
    Vec2   normal;

    Edge(const Vec2& p1_, const Vec2& p2_)
        : p1(p1_), p2(p2_), diff(p2 - p1), mag(diff.mag()), unitDiff(diff / mag),
          normal(unitDiff.y, unitDiff.x) {
        if (p1.x < p2.x) {
            min.x = p1.x;
            max.x = p2.x;
        }
        if (p1.y < p2.y) {
            min.y = p1.y;
            max.y = p2.y;
        }
    }

    // bool rayCast(const Vec2& pos, const Vec2& dir) {
    //     if ()
    // }
    // maybe when I can maths

    bool rayCast(const Vec2& pos) {
        if (pos.x < min.x || pos.x > max.x) return false; // outside x range
        if (pos.x == p1.x) return false; // perfect vertical allignment with one end
        if (diff.x == 0)
            return false; // if vertices form a verticle line a verticle line cannot intersect
        return (pos.x - p1.x) / diff.x * diff.y + p1.y > pos.y;
    }
};