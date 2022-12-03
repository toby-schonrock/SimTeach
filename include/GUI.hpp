#include "Sim.hpp"
#include "SFML/Graphics.hpp"

sf::Vector2f visualize(const Vec2& v);

enum drawFlags_ {
    drawFlags_None = 1 << 0,
    drawFlags_Springs = 1 << 1,
    drawFlags_Polygons = 1 << 2,
    drawFlags_Points = 1 << 3
};

class GUI{
    drawFlags_ drawFlags = drawFlags_None;

    void draw(sf::RenderWindow& window, Sim sim) {
        if (drawFlags & drawFlags_Springs) {
            for (Spring& spring: sim.springs) {
                spring.verts = {visualize(sim.points[spring.p1].pos), visualize(sim.points[spring.p2].pos)};
                window.draw(spring.verts.data(), 2, sf::Lines);
            }
        }
        if (drawFlags & drawFlags_Points) {
            for (Point& point: sim.points) point.draw(window);
        }
        if (drawFlags & drawFlags_Polygons) {
            for (Polygon& poly: sim.polys) poly.draw(window);
        }
    }

};