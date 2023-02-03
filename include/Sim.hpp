#pragma once

#include <algorithm>
#include <execution>
#include <functional>
#include <iostream>
#include <vector>

#include "EntityManager.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "SFML/Graphics.hpp"
#include "Spring.hpp"
#include "Vector2.hpp"

class Sim {
  public:
    EntityManager& entities;
    double         gravity;

    Sim(EntityManager& entities_, double gravity_ = 0) : entities(entities_), gravity(gravity_) {}

    void simFrame(double deltaTime) {
        // calculate spring force worth doing in parralel
        std::for_each(entities.springs.begin(), entities.springs.end(), [&](Spring& spring) {
            spring.springHandler(entities.points[spring.p1], entities.points[spring.p2]);
        });

        // update point positions
        std::for_each(entities.points.begin(), entities.points.end(),
                      [d = deltaTime, g = gravity](Point& point) { point.update(d, g); });

        // collide points with polygons
        for (const Polygon& poly: entities.polys) {
            for (Point& point: entities.points) {
                if (poly.isBounded(point.pos) &&
                    poly.isContained(point.pos)) // not sure if bounded check is still faster
                    poly.colHandler(point);
            }
        }
    }

    std::pair<std::size_t, double> findClosestPoint(const Vec2 pos) const {
        if (entities.points.empty())
            throw std::logic_error("Finding closest point with no points?!? ;)");
        double      closestDist = std::numeric_limits<double>::infinity();
        std::size_t closestPos  = 0;
        for (std::size_t i = 0; i != entities.points.size(); ++i) {
            Vec2   diff = pos - entities.points[i].pos;
            double dist = diff.x * diff.x + diff.y * diff.y;
            if (dist < closestDist) {
                closestDist = dist;
                closestPos  = i;
            }
        }
        return std::pair<std::size_t, double>(closestPos, std::sqrt(closestDist));
    }

    std::pair<std::size_t, double> findClosestSpring(const Vec2 pos) const {
        if (entities.springs.empty())
            throw std::logic_error("Finding closest spring with no springs?!? ;)");
        double      closestDist = std::numeric_limits<double>::infinity();
        std::size_t closestPos  = 0;
        for (std::size_t i = 0; i != entities.springs.size(); ++i) {
            double dist = pos.distToLine(entities.points[entities.springs[i].p1].pos,
                                         entities.points[entities.springs[i].p2].pos);
            if (dist < closestDist) {
                closestDist = dist;
                closestPos  = i;
            }
        }
        return std::pair<std::size_t, double>(closestPos, std::sqrt(closestDist));
    }

    static Sim softbody(EntityManager& entMan, const Vector2<std::size_t>& size, const Vec2& simPos,
                        float gravity, float gap, float springConst, float dampFact,
                        sf::Color color = sf::Color::Yellow) {
        Sim sim     = Sim(entMan);
        sim.gravity = gravity;

        entMan.polys.reserve(2);
        entMan.polys.push_back(Polygon::Square(Vec2(6, 10), -0.75));
        entMan.polys.push_back(Polygon::Square(Vec2(14, 10), 0.75));

        sim.entities.points.reserve(size.x * size.y);
        for (unsigned x = 0; x != size.x; ++x) {
            for (unsigned y = 0; y != size.y; ++y) {
                entMan.addPoint({Vec2(x, y) * gap + simPos, 1.0, color, false});
            }
        }

        for (std::size_t x = 0; x != size.x; ++x) {
            for (std::size_t y = 0; y != size.y; ++y) {
                std::size_t p = x + y * size.x;
                if (x < size.x - 1) {
                    if (y < size.y - 1) {
                        entMan.addSpring({springConst, dampFact,
                                          std::numbers::sqrt2 * static_cast<double>(gap), p,
                                          x + 1 + (y + 1) * size.x}); // down right
                    }
                    entMan.addSpring({springConst, dampFact, gap, p, x + 1 + (y)*size.x}); // right
                }
                if (y < size.y - 1) {
                    if (x > 0) {
                        entMan.addSpring({springConst, dampFact,
                                          std::numbers::sqrt2 * static_cast<double>(gap), p,
                                          x - 1 + (y + 1) * size.x}); // down left
                    }
                    entMan.addSpring({springConst, dampFact, gap, p, x + (y + 1) * size.x}); // down
                }
            }
        }
        return sim;
    }
};