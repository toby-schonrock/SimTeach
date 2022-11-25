#pragma once

#include <array>
#include <cstddef>
#include <vector>

#include "Point.hpp"
#include "Polygon.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/System/Vector2.hpp"
#include "Vector2.hpp"

struct Spring {
    std::array<sf::Vertex, 2> verts;
    float                     springConst;
    float                     dampFact;
    float                     stablePoint;
    std::size_t               p1;
    std::size_t               p2;
};

class Sim {
  public:
    std::vector<Polygon> polys;
    std::vector<Point>   points;
    std::vector<Spring>  springs;
    float                gravity;

    void draw(sf::RenderWindow& window) {

        for (Spring& spring: springs) {
            spring.verts = {visualize(points[spring.p1].pos), visualize(points[spring.p2].pos)};
            window.draw(spring.verts.data(), 2, sf::Lines);
        }
        for (Point& point: points) point.draw(window);
        for (Polygon& poly: polys) poly.draw(window);
    }

    void simFrame(double deltaTime) {
        // calculate spring force
        for (const Spring& spring: springs)
            springHandler(points[spring.p1], points[spring.p2], spring);

        // update point positions
        for (Point& point: points) point.update(deltaTime, gravity);

        // collide points with polygons
        for (const Polygon& poly: polys) {
            for (Point& point: points) {
                if (poly.isBounded(point.pos)) point.polyColHandler(poly);
            }
        }
    }

    void addPoint(const Point& p) { points.push_back(p); }

    void removePoint(const std::size_t& pos) {
        points.erase(points.begin() + static_cast<long long>(pos));
        remove_if(springs.begin(), springs.end(), [pos](const Spring& s) {
            return s.p1 == pos || s.p2 == pos;
        }); // NOLINT idk whats happening here
        for (Spring& s: springs) {
            if (s.p1 > pos) s.p1--;
            if (s.p2 > pos) s.p2--;
        }
    }

    static void springHandler(Point& p1, Point& p2, const Spring& spring) {
        Vec2   diff     = p1.pos - p2.pos; // broken out alot "yes this is faster! really like 3x"
        double diffMag  = diff.mag();
        Vec2   diffNorm = diff / diffMag;
        double ext      = diffMag - spring.stablePoint;
        double springf =
            -spring.springConst * ext; // -ke spring force and also if a diagonal increase
                                       // spring constant for stability // test
        double dampf = diffNorm.dot(p2.vel - p1.vel) * spring.dampFact; // damping force
        Vec2   force = (springf + dampf) * diffNorm;
        p1.f += force; // equal and opposite reaction
        p2.f -= force;
    }
};