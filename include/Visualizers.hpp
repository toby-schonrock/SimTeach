#pragma once

#include "Point.hpp"
#include "SFML/Graphics.hpp"
#include "Vector2.hpp"
#include <array>

sf::Vector2f visualize(const Vec2& v);

struct VisPoint {
  public:
    std::array<sf::Vertex, 4> v;

    VisPoint(const Point& p) {
        v[0].color = p.color;
        v[1].color = p.color;
        v[2].color = p.color;
        v[3].color = p.color;

        v[0].texCoords = sf::Vector2f{0, 0};
        v[1].texCoords = sf::Vector2f{15, 0};
        v[2].texCoords = sf::Vector2f{15, 15};
        v[3].texCoords = sf::Vector2f{0, 15};
    }

    void setColor(sf::Color color) {
        v[0].color = color;
        v[1].color = color;
        v[2].color = color;
        v[3].color = color;
    }
};

struct VisSpring {
  public:
    std::array<sf::Vertex, 2> v;
    VisSpring() = default;

    void setColor(sf::Color color) {
        v[0].color = color;
        v[1].color = color;
    }
};