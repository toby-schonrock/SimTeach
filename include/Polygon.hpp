#pragma once

#include <SFML/Graphics.hpp>
#include <Vector2.hpp>

sf::Vector2f visualize(const Vec2& v);

class Polygon {
    void boundsUp() {
        maxBounds = points[0];
        minBounds = points[0];
        for (std::size_t x = 1; x != pointCount; x++) {
            maxBounds.x = std::max(maxBounds.x, points[x].x);
            maxBounds.y = std::max(maxBounds.y, points[x].y);
            minBounds.x = std::min(minBounds.x, points[x].x);
            minBounds.y = std::min(minBounds.y, points[x].y);
        }
    }

  public:
    sf::ConvexShape   shape; // kind of annoying having to store this same with point TODO
    std::vector<Vec2> points;
    Vec2              maxBounds;
    Vec2              minBounds;
    std::size_t       pointCount;
    explicit Polygon(const std::vector<Vec2>& points_) // lvalue
        : points(points_), pointCount(points.size()) {
        shape.setPointCount(pointCount);
        boundsUp();
        for (std::size_t x = 0; x != pointCount; x++) shape.setPoint(x, visualize(points[x]));
    }

    explicit Polygon(std::vector<Vec2>&& points_) // rvalue
        : points(std::move(points_)), pointCount(points.size()) {
        shape.setPointCount(pointCount);
        boundsUp();
        for (std::size_t x = 0; x != pointCount; x++) shape.setPoint(x, visualize(points[x]));
    }

    bool isBounded(const Vec2& pos) const {
        return pos.x >= minBounds.x && pos.y >= minBounds.y && pos.x <= maxBounds.x &&
               pos.y <= maxBounds.y;
    }

    void draw(sf::RenderWindow& window) {
        // for (std::size_t x = 0; x != points.size(); x++) shape.setPoint(x, visualize(points[x]));
        // not nesecarry as it doesn't move
        window.draw(shape);
    }

    // static stuff
    static Polygon Square(const Vec2& pos, double tilt) {
        return Polygon({Vec2(5, 0.5) + pos, Vec2(-5, 0.5) + pos, Vec2(-5, -0.5 + tilt) + pos,
                        Vec2(5, -0.5 - tilt) + pos});
    }

    static Polygon Triangle(Vec2 pos) {
        return Polygon({Vec2(1, 1) + pos, Vec2(-1, 1) + pos, Vec2(0, -1) + pos});
    }
};