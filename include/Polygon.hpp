#pragma once

#include "Edge.hpp"
#include "Point.hpp"
#include "SFML/Graphics.hpp"
#include "Vector2.hpp"
#include "imgui.h"

sf::Vector2f visualize(const Vec2& v);

class Polygon {
  private:
    Vec2 maxBounds{};
    Vec2 minBounds{};

  public:
    std::vector<Edge>         edges{};
    sf::ConvexShape           shape;
    std::array<sf::Vertex, 2> line{};
    bool direction; // the way round the points go - true is anticlockwise

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
        isConvex(); // updates the direction variable ;)
    }

    // updates bounds of polygon
    void boundsUp() {
        for (const Edge& edge: edges) { // loop over all points
            const Vec2& vert = edge.p1();
            maxBounds.x   = std::max(maxBounds.x, vert.x);
            maxBounds.y   = std::max(maxBounds.y, vert.y);
            minBounds.x   = std::min(minBounds.x, vert.x);
            minBounds.y   = std::min(minBounds.y, vert.y);
        }
    }

    bool isBounded(const Vec2& pos) const {
        return pos.x >= minBounds.x && pos.y >= minBounds.y && pos.x <= maxBounds.x &&
                       pos.y <= maxBounds.y;
    }

    bool isContained(const Vec2& pos) const {
        bool contained = false;
            for (const Edge& edge: edges) 
                if (edge.rayCast(pos)) contained = !contained;
        return contained;
    }

    // checks if polygon is convex
    bool isConvex() {
        direction =
            std::signbit((edges.back().diff()).cross(edges.front().diff())); // first and last

        for (std::size_t i = 0; i != edges.size() - 1; ++i) {
            if (std::signbit(edges[i].diff().cross(edges[i + 1].diff())) != direction)
                return false; // checks up untill edge size - 1
        }
        return true;
    }

    // handle collision between point p and this
    void colHandler(Point& p) const {
        double closestDist = std::numeric_limits<double>::infinity();
        Vec2   closestPos;
        Vec2   normal;

        for (const Edge& edge: edges) {
            double dist = edge.distToPoint(p.pos);
            if (dist < closestDist) { // if new closest edge
                closestDist = dist;
                // note if clockwise (!dir) - normals are correct
                normal     = ((direction) ? 1 : -1) * edge.normal();
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

    // used for saving polygon to file as string
    friend std::ostream& operator<<(std::ostream& os, const Polygon& p) {
        if (!p.edges.empty()) {
            os << p.edges[0].p1().x << ' ' << p.edges[0].p1().y;
            for (std::size_t i = 1; i != p.edges.size(); ++i) {
                os << ' ' << p.edges[i].p1().x << ' ' << p.edges[i].p1().y;
            }
        }
        return os;
    }

    // used for creating polygon from string stream
    friend std::istream& operator>>(std::istream& is, Polygon& p) {
        std::vector<Vec2> verts{{}, {}, {}};
        safeStreamRead(is, verts[0]);
        safeStreamRead(is, verts[1]);
        safeStreamRead(is, verts[2]);
        while (is.good()) {
            verts.push_back(Vec2{});
            safeStreamRead(is, verts.back());
        }
        p = Polygon(verts);
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