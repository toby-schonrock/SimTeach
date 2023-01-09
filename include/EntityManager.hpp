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

    void updatePointVisPos(float radius) {
        for (std::size_t i = 0; i != points.size(); ++i) {
            pointVerts[i * 4].position = visualize(points[i].pos) + sf::Vector2f{-radius, -radius};
            pointVerts[i * 4 + 1].position =
                visualize(points[i].pos) + sf::Vector2f{radius, -radius};
            pointVerts[i * 4 + 2].position =
                visualize(points[i].pos) + sf::Vector2f{radius, radius};
            pointVerts[i * 4 + 3].position =
                visualize(points[i].pos) + sf::Vector2f{-radius, radius};
        }
    }

    void updateSpringVisPos() {
        for (std::size_t i = 0; i != springs.size(); ++i) {
            springVerts[i * 2].position     = visualize(points[springs[i].p1].pos);
            springVerts[i * 2 + 1].position = visualize(points[springs[i].p2].pos);
        }
    }
};