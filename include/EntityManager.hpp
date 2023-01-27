#pragma once

#include "Graph.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "SFML/Graphics.hpp"
#include "Spring.hpp"
#include "Vector2.hpp"

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

    void rmvPoint(const std::size_t& pos) {
        if (points.empty() || pos >= points.size()) {
            std::cout << "Attempted point - " << pos << "\n";
            throw std::logic_error("Asking to remove non existant point.");
        }

        std::size_t old    = points.size() - 1;
        auto        PStart = graphs.begin();
        auto        PEnd   = graphs.end();
        while (PStart < PEnd) {
            if ((*PStart).checkDeleteIndex(ObjectType::Point, pos)) { // remove invalid point graphs
                std::cout << "graph " << PStart - graphs.begin()
                          << " removed due to removal of point " << pos << "\n";
                *PStart = std::move(*(--PEnd));
            } else {
                (*PStart).updateIndex(ObjectType::Point, old, pos); // update indexs for move
                PStart++;
            }
        }
        graphs.erase(PEnd, graphs.end()); // finish the deleting of the graphs

        // swap point to the end and delete
        points[pos] = std::move(points.back()); // do the move
        points.pop_back();                      // delete
        pointVerts[pos * 4]     = std::move(pointVerts[pointVerts.size() - 4]);
        pointVerts[pos * 4 + 1] = std::move(pointVerts[pointVerts.size() - 3]);
        pointVerts[pos * 4 + 2] = std::move(pointVerts[pointVerts.size() - 2]);
        pointVerts[pos * 4 + 3] = std::move(pointVerts.back());
        pointVerts.erase(pointVerts.end() - 4, pointVerts.end());
        std::cout << "pointVert size: " << pointVerts.size() << "\n";

        // manual remove
        std::size_t end = springs.size();
        for (std::size_t curr = 0; curr < end; curr++) {
            if (springs[curr].p1 == pos || springs[curr].p2 == pos) { // delete
                std::cout << "spring " << curr << " removed"
                          << "\n";
                rmvSpring(curr);
                curr--; // to still check the moved one
                end--;
            } else {
                if (springs[curr].p1 == old)
                    springs[curr].p1 = pos; // spring was attatched to moved point
                if (springs[curr].p2 == old) springs[curr].p2 = pos;
            }
        }
    }

    void rmvSpring(const std::size_t& pos) {
        std::size_t old   = springs.size() - 1;
        auto        start = graphs.begin();
        auto        end   = graphs.end();
        while (start < end) {
            if ((*start).checkDeleteIndex(ObjectType::Spring,
                                          pos)) { // remove invalid spring graphs
                std::cout << "graph " << start - graphs.begin()
                          << " removed due to removal of spring " << pos << "\n";
                *start = std::move(*(--end));
            } else {
                (*start).updateIndex(ObjectType::Spring, old, pos); // update indexs for move
                start++;
            }
        }
        graphs.erase(end, graphs.end()); // finish the deleting of the graphs

        // swap spring to the end and delete
        springs[pos] = std::move(springs.back()); // do the move
        springs.pop_back();                       // delete
        springVerts[pos * 2]     = std::move(springVerts[springVerts.size() - 2]);
        springVerts[pos * 2 + 1] = std::move(springVerts.back());    // do the move
        springVerts.erase(springVerts.end() - 2, springVerts.end()); // delete
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
            springVerts[i * 2].position     = visualize(points[springs[i].p1].pos);
            springVerts[i * 2 + 1].position = visualize(points[springs[i].p2].pos);
        }
    }
};