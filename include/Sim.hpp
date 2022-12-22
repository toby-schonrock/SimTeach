#pragma once

#include <iostream>
#include <vector>

#include "Point.hpp"
#include "Polygon.hpp"
#include "SFML/Graphics.hpp"
#include "Vector2.hpp"

struct Spring {
    double      springConst;
    double      dampFact;
    double      stablePoint;
    std::size_t p1;
    std::size_t p2;

    void springHandler(Point& point1, Point& point2) const {
        Vec2 diff = point1.pos - point2.pos; // broken out alot "yes this is faster! really like 3x"
        double diffMag  = diff.mag();
        Vec2   diffNorm = diff / diffMag;
        double ext      = diffMag - stablePoint;
        double springf  = -springConst * ext; // -ke spring force and also if a diagonal increase
                                              // spring constant for stability // test
        double dampf = diffNorm.dot(point2.vel - point1.vel) * dampFact; // damping force
        Vec2   force = (springf + dampf) * diffNorm;
        point1.f += force; // equal and opposite reaction
        point2.f -= force;
    }
};

class Sim {
  public:
    std::vector<sf::Vertex> pointVerts;
    std::vector<sf::Vertex> springVerts;
    std::vector<Polygon>    polys;
    std::vector<Point>      points;
    std::vector<Spring>     springs;

    double gravity;

    void simFrame(double deltaTime) {
        // calculate spring force
        for (const Spring& spring: springs)
            spring.springHandler(points[spring.p1], points[spring.p2]);

        // update point positions
        for (Point& point: points) point.update(deltaTime, gravity);

        // collide points with polygons
        for (const Polygon& poly: polys) {
            for (Point& point: points) {
                if (poly.isBounded(point.pos) &&
                    poly.isContained(point.pos)) // not sure if bounded check is still faster
                    poly.colHandler(point);
            }
        }
    }

    void addPoint(const Point& p) {
        points.push_back(p);
        pointVerts.emplace_back(sf::Vector2f{}, p.color, sf::Vector2f{0, 0});
        pointVerts.emplace_back(sf::Vector2f{}, p.color, sf::Vector2f{300, 0});
        pointVerts.emplace_back(sf::Vector2f{}, p.color, sf::Vector2f{300, 300});
        pointVerts.emplace_back(sf::Vector2f{}, p.color, sf::Vector2f{0, 300});
    }

    // TODO this is slow(maybe)
    void rmvPoint(const std::size_t& pos) {
        if (points.empty() || pos >= points.size()) {
            std::cout << "Attempted point - " << pos << "\n";
            throw std::logic_error("Asking to remove non existant point.");
        }

        points.erase(points.begin() + static_cast<long long>(pos));
        pointVerts.erase(pointVerts.begin() + static_cast<long long>(pos * 4),
                         pointVerts.begin() + static_cast<long long>(pos * 4 + 4));

        // manual remove
        auto curr    = springs.begin();
        auto visCurr = springVerts.begin();
        auto last    = springs.end();
        auto visLast = springVerts.end();

        while (curr < last) {
            visCurr += 2; // can be done either way as will be reverted by move
            if (curr->p1 == pos || curr->p2 == pos) {
                *curr        = std::move(*(--last));
                *(--visCurr) = std::move(*(--visLast));
                *(--visCurr) = std::move(*(--visLast));
            } else {
                ++curr;
            }
        }
        // erase
        springs.erase(last, springs.end());
        springVerts.erase(visLast, springVerts.end());

        for (Spring& s: springs) {
            if (s.p1 > pos) --s.p1;
            if (s.p2 > pos) --s.p2;
        }
    }

    void addSpring(const Spring& s) {
        springs.push_back(s);
        springVerts.emplace_back();
        springVerts.emplace_back();
    }

    void rmvSpring(const std::size_t& pos) {
        springs.erase(springs.begin() + static_cast<long long>(pos));
        springVerts.erase(springVerts.begin() + static_cast<long long>(pos * 2),
                          springVerts.begin() + static_cast<long long>(pos * 2 + 2));
    }

    void updatePointVisPos() {
        for (std::size_t i = 0; i != points.size(); ++i) {
            pointVerts[i * 4].position =
                visualize(points[i].pos) + sf::Vector2f{-points[i].radius, -points[i].radius};
            pointVerts[i * 4 + 1].position =
                visualize(points[i].pos) + sf::Vector2f{points[i].radius, -points[i].radius};
            pointVerts[i * 4 + 2].position =
                visualize(points[i].pos) + sf::Vector2f{points[i].radius, points[i].radius};
            pointVerts[i * 4 + 3].position =
                visualize(points[i].pos) + sf::Vector2f{-points[i].radius, points[i].radius};
        }
    }

    void updateSpringVisPos() {
        for (std::size_t i = 0; i != springs.size(); ++i) {
            springVerts[i * 2].position     = visualize(points[springs[i].p1].pos);
            springVerts[i * 2 + 1].position = visualize(points[springs[i].p2].pos);
        }
    }

    void setPointColor(std::size_t i, sf::Color color) {
        i *= 4;
        pointVerts[i].color     = color;
        pointVerts[i + 1].color = color;
        pointVerts[i + 2].color = color;
        pointVerts[i + 3].color = color;
    }

    void resetPointColor(std::size_t i) {
        pointVerts[i * 4].color     = points[i].color;
        pointVerts[i * 4 + 1].color = points[i].color;
        pointVerts[i * 4 + 2].color = points[i].color;
        pointVerts[i * 4 + 3].color = points[i].color;
    }

    void setSpringColor(std::size_t i, sf::Color color) {
        i *= 2;
        springVerts[i].color     = color;
        springVerts[i + 1].color = color;
    }

    std::pair<std::size_t, double> findClosestPoint(const Vec2 pos) const {
        if (points.empty()) throw std::logic_error("Finding closest point with no points?!? ;)");
        double      closestDist = std::numeric_limits<double>::infinity();
        std::size_t closestPos  = 0;
        for (std::size_t i = 0; i != points.size(); ++i) {
            Vec2   diff = pos - points[i].pos;
            double dist = diff.x * diff.x + diff.y * diff.y;
            if (dist < closestDist) {
                closestDist = dist;
                closestPos  = i;
            }
        }
        return std::pair<std::size_t, double>(closestPos, std::sqrt(closestDist));
    }

    std::pair<std::size_t, double> findClosestSpring(const Vec2 pos) const {
        if (springs.empty()) throw std::logic_error("Finding closest spring with no springs?!? ;)");
        double      closestDist = std::numeric_limits<double>::infinity();
        std::size_t closestPos  = 0;
        for (std::size_t i = 0; i != springs.size(); ++i) {
            Vec2   springPos = 0.5 * (points[springs[i].p1].pos +
                                    points[springs[i].p2].pos); // average of both points
            Vec2   diff      = pos - springPos;
            double dist      = diff.x * diff.x + diff.y * diff.y;
            if (dist < closestDist) {
                closestDist = dist;
                closestPos  = i;
            }
        }
        return std::pair<std::size_t, double>(closestPos, std::sqrt(closestDist));
    }

    static Sim softbody(const Vector2<std::size_t>& size, const Vec2& simPos, float radius,
                        float gravity, float gap, float springConst, float dampFact,
                        sf::Color color = sf::Color::Yellow) {
        Sim sim     = Sim();
        sim.gravity = gravity;

        sim.polys.reserve(2);
        sim.polys.push_back(Polygon::Square(Vec2(6, 10), -0.75));
        sim.polys.push_back(Polygon::Square(Vec2(14, 10), 0.75));

        sim.points.reserve(size.x * size.y);
        for (unsigned x = 0; x != size.x; ++x) {
            for (unsigned y = 0; y != size.y; ++y) {
                sim.addPoint({Vec2(x, y) * gap + simPos, 1.0, radius, color});
            }
        }

        for (std::size_t x = 0; x != size.x; ++x) {
            for (std::size_t y = 0; y != size.y; ++y) {
                std::size_t p = x + y * size.x;
                if (x < size.x - 1) {
                    if (y < size.y - 1) {
                        sim.addSpring({springConst, dampFact,
                                       std::numbers::sqrt2 * static_cast<double>(gap), p,
                                       x + 1 + (y + 1) * size.x}); // down right
                    }
                    sim.addSpring({springConst, dampFact, gap, p, x + 1 + (y)*size.x}); // right
                }
                if (y < size.y - 1) {
                    if (x > 0) {
                        sim.addSpring({springConst, dampFact,
                                       std::numbers::sqrt2 * static_cast<double>(gap), p,
                                       x - 1 + (y + 1) * size.x}); // down left
                    }
                    sim.addSpring({springConst, dampFact, gap, p, x + (y + 1) * size.x}); // down
                }
            }
        }
        return sim;
    }
};