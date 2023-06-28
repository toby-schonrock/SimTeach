#pragma once

#include "Fundamentals/Vector2.hpp"
#include "Graph.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "SFML/Graphics.hpp"
#include "Spring.hpp"
#include <cstddef>
#include <filesystem>
#include <iostream>

struct ObjectEnabled {
    bool points;
    bool springs;
    bool polygons;
};

class EntityManager {
  public:
    std::vector<sf::Vertex> pointVerts;
    std::vector<sf::Vertex> springVerts;
    std::vector<Polygon>    polys;
    std::vector<Point>      points;
    std::vector<Spring>     springs;
    std::vector<Graph>      graphs;

    void addPoint(const Point& p) {
        points.push_back(p);
        pointVerts.emplace_back(sf::Vector2f{}, p.color, sf::Vector2f{0, 0});
        pointVerts.emplace_back(sf::Vector2f{}, p.color, sf::Vector2f{300, 0});
        pointVerts.emplace_back(sf::Vector2f{}, p.color, sf::Vector2f{300, 300});
        pointVerts.emplace_back(sf::Vector2f{}, p.color, sf::Vector2f{0, 300});
    }

    void addSpring(const Spring& s) {
        springs.push_back(s);
        springVerts.emplace_back();
        springVerts.emplace_back();
    }

    void rmvPoint(PointId pos) {
        PointId old = static_cast<PointId>(points.size() - 1);
        if (points.empty() || pos > old) {
            throw std::logic_error("Asking to remove non existant point - " +
                                   std::to_string(static_cast<std::size_t>(pos)));
        }

        // swap point to the end and delete
        points[static_cast<std::size_t>(pos)] = std::move(points.back()); // do the move
        points.pop_back();                                                // delete
        pointVerts[static_cast<std::size_t>(pos) * 4] =
            std::move(pointVerts[pointVerts.size() - 4]);
        pointVerts[static_cast<std::size_t>(pos) * 4 + 1] =
            std::move(pointVerts[pointVerts.size() - 3]);
        pointVerts[static_cast<std::size_t>(pos) * 4 + 2] =
            std::move(pointVerts[pointVerts.size() - 2]);
        pointVerts[static_cast<std::size_t>(pos) * 4 + 3] = std::move(pointVerts.back());
        pointVerts.resize(pointVerts.size() - 4);

        auto GCurr = graphs.begin();
        auto GEnd  = graphs.end();
        while (GCurr < GEnd) {
            if ((*GCurr).checkDeleteIndex(pos)) { // remove invalid point graphs
                std::cout << "Graph of " << GCurr->getYLabel()
                          << " removed due to removal of point " << static_cast<std::size_t>(pos)
                          << "\n";
                *GCurr = std::move(*(--GEnd));
            } else {
                (*GCurr).updateIndex(old, pos); // update indexs for move
                ++GCurr;
            }
        }
        graphs.erase(GEnd, graphs.end()); // finish the deleting of the graphs

        // manual remove
        std::size_t SEnd = springs.size();
        for (std::size_t SCurr = 0; SCurr < SEnd; SCurr++) {
            if (springs[SCurr].p1 == pos || springs[SCurr].p2 == pos) { // delete
                rmvSpring(static_cast<SpringId>(SCurr));
                SCurr--; // to still check the moved one
                SEnd--;
            } else {
                if (springs[SCurr].p1 == old)
                    springs[SCurr].p1 = pos; // spring was attatched to moved point
                if (springs[SCurr].p2 == old) springs[SCurr].p2 = pos;
            }
        }
    }

    void rmvSpring(SpringId pos) {
        SpringId old = static_cast<SpringId>(springs.size() - 1);
        if (springs.empty() || pos > old) {
            throw std::logic_error("Asking to remove non existant spring - " +
                                   std::to_string(static_cast<std::size_t>(pos)));
        }

        // swap spring to the end and delete
        springs[static_cast<std::size_t>(pos)] = std::move(springs.back()); // do the move
        springs.pop_back();                                                 // delete
        springVerts[static_cast<std::size_t>(pos) * 2] =
            std::move(springVerts[springVerts.size() - 2]);
        springVerts[static_cast<std::size_t>(pos) * 2 + 1] =
            std::move(springVerts.back());          // do the move
        springVerts.resize(springVerts.size() - 2); // delete

        auto GStart = graphs.begin();
        auto GEnd   = graphs.end();
        while (GStart < GEnd) {
            if ((*GStart).checkDeleteIndex(pos)) { // remove invalid spring graphs
                std::cout << "Graph of " << GStart->getYLabel()
                          << " removed due to removal of spring " << static_cast<std::size_t>(pos)
                          << "\n";
                *GStart = std::move(*(--GEnd));
            } else {
                (*GStart).updateIndex(old, pos); // update indexs for move
                GStart++;
            }
        }
        graphs.erase(GEnd, graphs.end()); // finish the deleting of the graphs
    }

    void updatePointVisPos(float radius) {
        for (std::size_t i = 0; i != points.size(); ++i) {
            sf::Vector2f pos               = visualize(points[i].pos);
            pointVerts[i * 4].position     = pos + sf::Vector2f{-radius, -radius};
            pointVerts[i * 4 + 1].position = pos + sf::Vector2f{radius, -radius};
            pointVerts[i * 4 + 2].position = pos + sf::Vector2f{radius, radius};
            pointVerts[i * 4 + 3].position = pos + sf::Vector2f{-radius, radius};
        }
    }

    void updateSpringVisPos() {
        for (std::size_t i = 0; i != springs.size(); ++i) {
            springVerts[i * 2].position     = visualize(points[static_cast<std::size_t>(springs[i].p1)].pos);
            springVerts[i * 2 + 1].position = visualize(points[static_cast<std::size_t>(springs[i].p2)].pos);
        }
    }
};