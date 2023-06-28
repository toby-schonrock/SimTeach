#pragma once

#include <algorithm>
#include <cstddef>
#include <filesystem>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Edge.hpp"
#include "EntityManager.hpp"
#include "Fundamentals/Vector2.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "SFML/Graphics.hpp"
#include "Spring.hpp"

extern const std::filesystem::path Previous;
static const std::string PointHeaders{"point-id fixed posx posy velx vely mass color(rgba)"};
static const std::string SpringHeaders =
    "spring-id spring-const natural-length damping-factor point1 point2";
static const std::string PolyHeaders = "polygon-verts: x y ...";

class Sim {
  public:
    EntityManager& entities;
    double         gravity;

    Sim(EntityManager& entities_, double gravity_ = 0) : entities(entities_), gravity(gravity_) {
        std::filesystem::create_directory("sims");
    }

    void simFrame(double deltaTime) {
        // calculate spring force worth doing in parralel
        std::for_each(entities.springs.begin(), entities.springs.end(), [&](Spring& spring) {
            spring.springHandler(entities.points[static_cast<std::size_t>(spring.p1)],
                                 entities.points[static_cast<std::size_t>(spring.p2)]);
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

    std::pair<PointId, double> findClosestPoint(const Vec2 pos) const {
        if (entities.points.empty())
            throw std::logic_error("Finding closest point with no points?!? ;)");
        double  closestDist = std::numeric_limits<double>::infinity();
        PointId closestPos{};
        for (std::size_t i = 0; i != entities.points.size(); ++i) {
            Vec2   diff = pos - entities.points[i].pos;
            double dist = diff.x * diff.x + diff.y * diff.y;
            if (dist < closestDist) {
                closestDist = dist;
                closestPos  = static_cast<PointId>(i);
            }
        }
        return std::pair<PointId, double>(closestPos, std::sqrt(closestDist));
    }

    std::pair<SpringId, double> findClosestSpring(const Vec2 pos) const {
        if (entities.springs.empty())
            throw std::logic_error("Finding closest spring with no springs?!? ;)");
        double   closestDist = std::numeric_limits<double>::infinity();
        SpringId closestPos{};
        for (std::size_t i = 0; i != entities.springs.size(); ++i) {
            double dist = pos.distToLine(
                entities.points[static_cast<std::size_t>(entities.springs[i].p1)].pos,
                entities.points[static_cast<std::size_t>(entities.springs[i].p2)].pos);
            if (dist < closestDist) {
                closestDist = dist;
                closestPos  = static_cast<SpringId>(i);
            }
        }
        return std::pair<SpringId, double>(closestPos, std::sqrt(closestDist));
    }

    void reset() { load(Previous, true, {true, true, true}, false); }

    void load(std::filesystem::path path, bool replace, ObjectEnabled enabled,
              bool deleteGraphs = true) {
        path.make_preferred();
        if (replace) {
            if (deleteGraphs) entities.graphs.clear();
            entities.points.clear();
            entities.pointVerts.clear();
            entities.springs.clear();
            entities.springVerts.clear();
            entities.polys.clear();
        }
        std::size_t pointOffset = entities.points.size();

        std::ifstream file{path, std::ios_base::in};
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file \"" + path.string() + '"');
        }

        std::string line;
        // points
        std::getline(file, line);
        if (line != PointHeaders)
            throw std::runtime_error("Point headers invalid: \n is - " + line + "\n should be - " +
                                     PointHeaders);

        std::stringstream ss;
        std::size_t       index = 0;
        while (true) {
            std::getline(file, line);
            if (checkIfHeader(SpringHeaders, line)) break;
            if (enabled.points) {
                Point point{};
                ss = std::stringstream(line);
                std::size_t temp;
                safeStreamRead(ss, temp);
                if (temp != index)
                    throw std::runtime_error("Non continous point indicie - " + line);
                safeStreamRead(ss, point);
                ++index;
                entities.addPoint(point);
            }
        }

        index = 0;
        while (true) {
            std::getline(file, line);
            if (checkIfHeader(PolyHeaders, line)) break;
            if (enabled.springs) {
                ss = std::stringstream(line);
                Spring      spring{};
                std::size_t temp;
                safeStreamRead(ss, temp);
                if (temp != index)
                    throw std::runtime_error("Non continous spring indicie - " + line);
                safeStreamRead(ss, spring);
                spring.p1 = static_cast<PointId>(static_cast<std::size_t>(spring.p1) + pointOffset);
                spring.p2 = static_cast<PointId>(static_cast<std::size_t>(spring.p2) + pointOffset);
                ++index;
                entities.addSpring(spring);
            }
        }

        while (!file.eof()) {
            std::getline(file, line);
            if (enabled.polygons) {
                ss = std::stringstream(line);
                if (ss.good()) { // deal with emtpy new lines at end
                    Polygon poly{};
                    safeStreamRead(ss, poly);
                    entities.polys.push_back(poly);
                }
            }
        }
    }

    void save(std::filesystem::path path, ObjectEnabled enabled) const {
        path.make_preferred();
        std::ofstream file{path, std::ios_base::out};
        if (!file.is_open()) {
            throw std::runtime_error("Falied to open fstream \n");
        }

        file << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10);
        file << PointHeaders << "\n";
        if (enabled.points) {
            for (std::size_t i = 0; i != entities.points.size(); ++i) {
                file << i << ' ' << entities.points[i] << "\n";
            }
        }
        file << SpringHeaders << "\n";
        if (enabled.springs) {
            for (std::size_t i = 0; i != entities.springs.size(); ++i) {
                file << i << ' ' << entities.springs[i] << "\n";
            }
        }
        file << PolyHeaders;
        if (enabled.polygons) {
            for (const Polygon& p: entities.polys) {
                if (!p.edges.empty()) file << "\n" << p;
            }
        }
    }

    static bool checkIfHeader(const std::string& header, const std::string& line) {
        if (header.size() != line.size()) return false;
        for (std::size_t i = 0; i != line.size(); ++i) {
            if (header[i] != line[i]) return false;
        }
        return true;
    }

    static Sim softbody(EntityManager& entities, const Vector2<std::size_t>& size,
                        const Vec2& simPos, float gravity, float gap, float springConst,
                        float dampFact, sf::Color color = sf::Color::Yellow) {
        Sim sim     = Sim(entities);
        sim.gravity = gravity;

        entities.polys.reserve(2);
        entities.polys.push_back(Polygon::Square(Vec2(1, 0), -0.75));
        entities.polys.push_back(Polygon::Square(Vec2(9, 0), 0.75));

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
};