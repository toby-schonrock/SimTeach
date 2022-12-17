#pragma once

#include "Vector2.hpp"
#include "SFML/Graphics.hpp"

sf::Vector2f visualize(const Vec2& v);

class Point {
  public:
    sf::CircleShape shape;
    sf::Color       color;
    Vec2            pos;
    Vec2            vel{0, 0}; // set to 0,0
    Vec2            f;
    double          mass = 1.0;

    Point() = default;

    Point(const Vec2& pos_, double mass_, float radius, sf::Color color_)
        : shape(sf::CircleShape()), color(color_), pos(pos_), mass(mass_) {
        shape.setFillColor(color);
        shape.setPosition(visualize(pos));
        shape.setRadius(radius);
        shape.setOrigin(visualize(Vec2(radius, radius)));
    }

    void resetColor() { shape.setFillColor(color); }

    void draw(sf::RenderWindow& window) {
        shape.setPosition(visualize(pos));
        window.draw(shape);
    }

    void update(double deltaTime, double gravity) {
        vel += (f / mass + Vec2(0, 1) * gravity) *
               deltaTime; // TODO euler integration could be improved
        pos += vel * deltaTime;
        f = Vec2();
    }
};