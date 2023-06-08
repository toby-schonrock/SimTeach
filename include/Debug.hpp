#pragma once

#include "Polygon.hpp"
#include "Edge.hpp"
#include "SFML/Graphics.hpp"
#include "Fundamentals/Vector2.hpp"

sf::Vector2f visualize(const Vec2& v);

inline void debugLine(const Vec2& p1, const Vec2& p2, sf::RenderWindow& window, sf::Color color) {
    std::array<sf::Vertex, 2> line{sf::Vertex(visualize(p1), color),
                                   sf::Vertex(visualize(p2), color)};
    window.draw(line.data(), 2, sf::Lines);
}

inline void debugPoint(const Vec2& pos, float radius, sf::RenderWindow& window, sf::Color color) {
    sf::CircleShape shape{radius};
    shape.setFillColor(color);
    shape.setPosition(visualize(pos));
    shape.setOrigin({radius, radius});
    window.draw(shape);
}

inline void debugNormals(const Polygon& poly, sf::RenderWindow& window, sf::Color color, double size = 1) {
    for (const Edge& e: poly.edges) {
        debugLine(e.p1() + 0.5 * e.diff(),
                  e.p1() + 0.5 * e.diff() + ((poly.direction) ? 1 : -1) * e.normal() * size, window,
                  color);
    }
}