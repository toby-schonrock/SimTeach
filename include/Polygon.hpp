#pragma once

#include "Edge.hpp"
#include "Point.hpp"
#include "SFML/Graphics.hpp"
#include "Vector2.hpp"
#include "imgui.h"

sf::Vector2f visualize(const Vec2& v);

// so this polygon is very different can do wierd stuff like draw when it only has two edges
// also at all times this polygon must never have one edge (the edges must always loop)

class Polygon {
  private:
    Vec2 maxBounds{};
    Vec2 minBounds{};

  public:
    std::vector<Edge>         edges{};
    sf::ConvexShape           shape;
    std::array<sf::Vertex, 2> line{};
    bool direction; // the way round the points go - true is anticlockwise (i think)

    explicit Polygon() = default;

    explicit Polygon(const std::vector<Vec2>& points) {
        if (points.size() == 1)
            throw std::logic_error("Polygon cannot be constructed with 1 point");

        shape.setPointCount(points.size());
        for (std::size_t i = 0; i != points.size() - 1; i++) {
            edges.push_back({points[i], points[i + 1]});
            shape.setPoint(i, visualize(points[i]));
        }
        edges.push_back({points[points.size() - 1], points[0]}); // for last one
        shape.setPoint(points.size() - 1, visualize(points[points.size() - 1]));
        boundsUp();
        if (!isConvex())
            throw std::logic_error(
                "Polygon points given are not convex"); // also updates the direction variable ;)
    }

    void boundsUp() {
        // TODO doesn't take advantage of mins/maxs already calulated in edges
        for (const Edge& edge: edges) { // loop over all points
            const Vec2& p      = edge.p1();
            maxBounds.x = std::max(maxBounds.x, p.x);
            maxBounds.y = std::max(maxBounds.y, p.y);
            minBounds.x = std::min(minBounds.x, p.x);
            minBounds.y = std::min(minBounds.y, p.y);
        }
    }

    void addEdge(const Vec2& pos) {
        if (edges.empty())
            throw std::logic_error("cant addEdge if empty poly : poly must have two edges");
        edges.back().p2(pos);
        edges.emplace_back(pos, edges.front().p1());
        shape.setPointCount(edges.size());
        shape.setPoint(edges.size() - 1, visualize(pos));
        boundsUp(); // keep bounds up to date
    }

    void rmvEdge() {
        if (edges.empty()) throw std::logic_error("cant rmvEdge if polygon empty");
        if (edges.size() == 2) {
            edges.clear();
        } else {
            edges.pop_back();
            edges.back().p2(edges.front().p1());
            shape.setPointCount(edges.size());
        }
        boundsUp(); // keep bounds up to date
    }

    bool isBounded(const Vec2& pos) const {
        bool bounded = pos.x >= minBounds.x && pos.y >= minBounds.y && pos.x <= maxBounds.x &&
               pos.y <= maxBounds.y;
        ImGui::Text("Bounded - %i", bounded);
        return bounded;
    }

    bool isContained(const Vec2& pos) const {
        int  i{};
        bool contained = false;
        for (const Edge& e: edges) {
            if (e.rayCast(pos)) {
                contained = !contained;
                ++i;
            }
        }
        if (ImGui::Begin("debug")) {
            ImGui::Text("count - %i", i);
            ImGui::End();
        }
        return contained;
    }

    bool isConvex() {
        direction =
            std::signbit((edges.back().diff()).cross(edges.front().diff())); // first and last

        for (std::size_t i = 0; i != edges.size() - 1; ++i) {
            if (std::signbit(edges[i].diff().cross(edges[i + 1].diff())) != direction)
                return false; // checks up untill edge size - 1
        }
        return true;
    }

    void colHandler(Point& p) const {
        double closestDist = std::numeric_limits<double>::infinity();
        Vec2   closestPos;
        Vec2   normal; // p.pos is just a place holder (TODO bad) - saves making a copy of
                       // normal for every edge

        for (const Edge& e: edges) {
            double dist = e.distToPoint(p.pos);
            if (dist < closestDist) { // if new closest edge
                closestDist = dist;
                // note if clockwise (!dir) - normals are correct
                normal     = ((direction) ? 1 : -1) * e.normal();
                closestPos = p.pos + normal * dist;
            }
        }
        p.pos = closestPos;
        p.vel -= (2 * normal.dot(p.vel) * normal); // vector reflection formula
    }

    void draw(sf::RenderWindow& window, bool update) {
        if (edges.empty()) throw std::logic_error("cant draw poly if has no edges");
        if (edges.size() > 2) {
            if (update) {
                shape.setPointCount(edges.size());
                for (std::size_t i = 0; i != edges.size(); i++) {
                    shape.setPoint(i, visualize(edges[i].p1()));
                }
            }

            window.draw(shape);
        } else if (edges.size() == 2) {
            if (update) {
                line[0].position = visualize(edges[0].p1());
                line[1].position = visualize(edges[1].p1());
            }
            window.draw(line.data(), 2, sf::Lines);
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Polygon& p) {
        if (!p.edges.empty()) {
            os << p.edges[0].p1().x << ' ' << p.edges[0].p1().y;
            for (std::size_t e = 1; e != p.edges.size(); ++e) {
                os << ' ' << p.edges[e].p1().x << ' ' << p.edges[e].p1().y;
            }
        }
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Polygon& p) {
        Vec2 v1, v2, v3;
        safeStreamRead(is, v1);
        safeStreamRead(is, v2);
        safeStreamRead(is, v3);
        p = Polygon({v1, v2, v3});
        while (is.good()) {
            safeStreamRead(is, v1);
            p.addEdge(v1);
        }
        if (p.isConvex() == false)
            throw std::runtime_error("Polygon vertices do not form a convex polygon");
        return is;
    }

    // static stuff
    static Polygon Square(const Vec2& pos, double tilt) {
        return Polygon({pos, Vec2(10, 0) + pos, Vec2(10, 1 + tilt) + pos, Vec2(0, 1 - tilt) + pos});
    }

    static Polygon Triangle(Vec2 pos) {
        return Polygon({Vec2(1, 1) + pos, Vec2(-1, 1) + pos, Vec2(0, -1) + pos});
    }
};