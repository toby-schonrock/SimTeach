#pragma once

#include <X11/Xmd.h>
#include <algorithm>
#include <array>
#include <cstddef>
#include <execution>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "Edge.hpp"
#include "EntityManager.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "SFML/Graphics.hpp"
#include "Spring.hpp"
#include "Vector2.hpp"

static const std::string pointHeaders = "point-id,fixed,posx,posy,velx,vely,mass,color(rgba)";
static const std::string springHeaders =
    "spring-id,spring-const,natural-length,damping-factor,point1,point2";
static const std::string polyHeaders = "polygon:verts...";

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

    static Sim softbody(EntityManager& entities, const Vector2<std::size_t>& size,
                        const Vec2& simPos, float gravity, float gap, float springConst,
                        float dampFact, sf::Color color = sf::Color::Yellow) {
        Sim sim     = Sim(entities);
        sim.gravity = gravity;

        entities.polys.reserve(2);
        entities.polys.push_back(Polygon::Square(Vec2(6, 10), -0.75));
        entities.polys.push_back(Polygon::Square(Vec2(14, 10), 0.75));

        sim.entities.points.reserve(size.x * size.y);
        for (unsigned x = 0; x != size.x; ++x) {
            for (unsigned y = 0; y != size.y; ++y) {
                entities.addPoint({Vec2(x, y) * gap + simPos, 1.0, color, false});
            }
        }

        for (std::size_t x = 0; x != size.x; ++x) {
            for (std::size_t y = 0; y != size.y; ++y) {
                std::size_t p = x + y * size.x;
                if (x < size.x - 1) {
                    if (y < size.y - 1) {
                        entities.addSpring({springConst, dampFact,
                                            std::numbers::sqrt2 * static_cast<double>(gap), p,
                                            x + 1 + (y + 1) * size.x}); // down right
                    }
                    entities.addSpring(
                        {springConst, dampFact, gap, p, x + 1 + (y)*size.x}); // right
                }
                if (y < size.y - 1) {
                    if (x > 0) {
                        entities.addSpring({springConst, dampFact,
                                            std::numbers::sqrt2 * static_cast<double>(gap), p,
                                            x - 1 + (y + 1) * size.x}); // down left
                    }
                    entities.addSpring(
                        {springConst, dampFact, gap, p, x + (y + 1) * size.x}); // down
                }
            }
        }
        return sim;
    }

    void load(const std::string& name, bool replace) {
        std::filesystem::path p = name + ".csv";
        if (replace) {
            entities.graphs.clear();
            entities.points.clear();
            entities.pointVerts.clear();
            entities.springs.clear();
            entities.springVerts.clear();
            entities.polys.clear();
        }
        std::size_t offsetPoint  = entities.points.size();
        std::size_t offsetSpring = entities.springs.size();
        std::size_t offsetPoly   = entities.polys.size();

        std::ifstream file{p.make_preferred(), std::ios_base::in};
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file \"" + p.make_preferred().string() + '"');
        }

        std::string line;
        // points
        std::getline(file, line);
        if (line != pointHeaders)
            throw std::runtime_error("Point headers invalid: \n is - " + line + "\n should be - " +
                                     pointHeaders);
        while (true) {
            std::getline(file, line);
            if (checkIfHeader(springHeaders, line)) break;
            pointFromLine(line);
            // std::cout << line << "\n";
        }
    }

    static void pointFromLine(std::string_view line) {
        std::array<std::string_view, 11> pointParts;
        std::size_t                      partCount = 0;
        std::size_t                      subLength = 0;
        for (std::size_t i = 0; i != line.size(); ++i) {
            if (line[i] == ',') {
                if (partCount == pointParts.size() - 1)
                    throw std::runtime_error("Too many columns in point row: " +
                                             std::string(line));
                pointParts[partCount] = line.substr(i - subLength, subLength);
                subLength             = 0;
                ++partCount;
            } else {
                ++subLength;
            }
        }
        pointParts[partCount] = line.substr(line.size() - subLength, subLength);
        if (partCount < pointParts.size() - 1)
            throw std::runtime_error("Too few columns in point row: " +
                                             std::string(line));
        std::stof(pointParts[1])
    }

    bool checkIfHeader(const std::string& header, const std::string& line) {
        if (header.size() != line.size()) return false;
        for (std::size_t i = 0; i != line.size(); ++i) {
            if (header[i] != line[i]) return false;
        }
        return true;
    }

    void save(const std::string& name) const {
        std::filesystem::path path = name + ".csv";
        std::ofstream         file{path.make_preferred(), std::ios_base::out};
        if (!file.is_open()) {
            throw std::runtime_error("Falied to open fstream \n");
        }

        file << std::fixed << std::setprecision(10);
        file << pointHeaders << "\n";
        for (std::size_t i = 0; i != entities.points.size(); ++i) {
            const Point& p = entities.points[i];
            file << i << "," << p.fixed << "," << p.pos.x << "," << p.pos.y << "," << p.vel.x << ","
                 << p.vel.y << "," << p.mass << "," << std::to_string(p.color.r) << ","
                 << std::to_string(p.color.g) << "," << std::to_string(p.color.b) << ","
                 << std::to_string(p.color.a) << "\n";
        }
        file << springHeaders << "\n";
        for (std::size_t i = 0; i != entities.springs.size(); ++i) {
            const Spring& s = entities.springs[i];
            file << i << "," << s.springConst << "," << s.naturalLength << "," << s.dampFact << ","
                 << s.p1 << "," << s.p2 << "\n";
        }
        file << polyHeaders << "\n";
        for (std::size_t i = 0; i != entities.polys.size(); ++i) {
            const Polygon& p = entities.polys[i];
            if (!p.edges.empty()) {
                file << p.edges[0].p1().x << "," << p.edges[0].p1().y;
                for (std::size_t e = 1; e != p.edges.size(); ++e) {
                    file << "," << p.edges[e].p1().x << "," << p.edges[e].p1().y;
                }
                file << "\n";
            }
        }
    }
};