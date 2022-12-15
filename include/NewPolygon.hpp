#pragma once

#include <Edge.hpp>
#include <SFML/Graphics.hpp>
#include <Vector2.hpp>

sf::Vector2f visualize(const Vec2& v);

class NewPolygon {
    void boundsUp() {
        // TODO doesn't take advantage of mins/maxs already calulates
        for (const Edge& edge: edges) { // loop over all points
            maxBounds.x = std::max(maxBounds.x, edge.p1.x);
            maxBounds.y = std::max(maxBounds.y, edge.p1.y);
            minBounds.x = std::min(minBounds.x, edge.p1.x);
            minBounds.y = std::min(minBounds.y, edge.p1.y);
        }
    }

  public:
    sf::ConvexShape   shape; // kind of annoying having to store this same with point TODO
    std::vector<Edge> edges;
    Vec2              maxBounds{};
    Vec2              minBounds{};
    explicit NewPolygon(const std::vector<Vec2>& points) {
        shape.setPointCount(points.size());
        for (std::size_t i = 0; i != points.size() - 1; i++) {
            edges.push_back({points[i], points[i + 1]});
            shape.setPoint(i, visualize(points[i]));
        }
        edges.push_back({points[points.size() - 1], points[0]}); // for last one
        shape.setPoint(points.size() - 1, visualize(points[points.size() - 1]));
        boundsUp();
    }

    bool isBounded(const Vec2& pos) const {
        return pos.x >= minBounds.x && pos.y >= minBounds.y && pos.x <= maxBounds.x &&
               pos.y <= maxBounds.y;
    }

    void draw(sf::RenderWindow& window) { window.draw(shape); }

    // static stuff
    static NewPolygon Square(const Vec2& pos, double tilt) {
        return NewPolygon({Vec2(5, 0.5) + pos, Vec2(-5, 0.5) + pos, Vec2(-5, -0.5 + tilt) + pos,
                           Vec2(5, -0.5 - tilt) + pos});
    }

    static NewPolygon Triangle(Vec2 pos) {
        return NewPolygon({Vec2(1, 1) + pos, Vec2(-1, 1) + pos, Vec2(0, -1) + pos});
    }
};