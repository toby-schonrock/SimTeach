#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <vector>

#include "Point.hpp"
#include "Polygon.hpp"
#include "SFML/Graphics.hpp"
#include "Vector2.hpp"

struct Spring {
    std::array<sf::Vertex, 2> verts; // TODO perhaps store inderictly to save size (more cacheing)
    double                    springConst;
    double                    dampFact;
    double                    stablePoint;
    std::size_t               p1;
    std::size_t               p2;
};

class Sim {
  public:
    std::vector<Polygon> polys;
    std::vector<Point>   points;
    std::vector<Spring>  springs;
    double               gravity;

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
        std::erase_if(springs, [pos](const Spring& s) { return s.p1 == pos || s.p2 == pos; });
        for (Spring& s: springs) {
            if (s.p1 > pos) --s.p1;
            if (s.p2 > pos) --s.p2;
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

    static Sim softbody(const Vector2<std::size_t>& size, const Vec2& simPos, float radius,
                        float gravity, float gap, float springConst, float dampFact) {
        Sim sim     = Sim();
        sim.gravity = gravity;

        sim.polys.reserve(3);
        sim.polys.push_back(Polygon::Square(Vec2(6, 10), -0.75));
        sim.polys.push_back(Polygon::Square(Vec2(14, 10), 0.75));

        sim.points.reserve(size.x * size.y);
        for (unsigned x = 0; x != size.x; ++x) {
            for (unsigned y = 0; y != size.y; ++y) {
                sim.addPoint({Vec2(x, y) * gap + simPos, 1.0, radius});
            }
        }

        for (std::size_t x = 0; x != size.x; ++x) {
            for (std::size_t y = 0; y != size.y; ++y) {
                std::size_t p = x + y * size.x;
                if (x < size.x - 1) {
                    if (y < size.y - 1) {
                        sim.springs.push_back({{},
                                               springConst,
                                               dampFact,
                                               std::numbers::sqrt2 * static_cast<double>(gap),
                                               p,
                                               x + 1 + (y + 1) * size.x}); // down right
                    }
                    sim.springs.push_back(
                        {{}, springConst, dampFact, gap, p, x + 1 + (y)*size.x}); // right
                }
                if (y < size.y - 1) {
                    if (x > 0) {
                        sim.springs.push_back({{},
                                               springConst,
                                               dampFact,
                                               std::numbers::sqrt2 * static_cast<double>(gap),
                                               p,
                                               x - 1 + (y + 1) * size.x}); // down left
                    }
                    sim.springs.push_back(
                        {{}, springConst, dampFact, gap, p, x + (y + 1) * size.x}); // down
                }
            }
        }
        return sim;
    }
};