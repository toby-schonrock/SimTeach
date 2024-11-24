#pragma once

#include "SFML/Graphics.hpp"
#include "physenv/Engine.hpp"

struct ObjectEnabled {
    bool points;
    bool springs;
    bool polygons;
};

// TODO arguably all the drawing n shit should be in here

inline sf::Vector2f visualize(const physenv::Vec2& v) {
    return {static_cast<float>(v.x), static_cast<float>(v.y)};
}

class EntityManager {
  public:
    physenv::Engine engine;

  private:
    // wether color has been changed since last draw (avoids updating colors when not needed)
    bool                                              pointColorChanged  = true;
    bool                                              springColorChanged = true;
    std::unordered_map<physenv::PointRef, sf::Color>  pointColors;
    std::unordered_map<physenv::SpringRef, sf::Color> springColors;
    std::vector<sf::Vertex>                           pointVerts;
    std::vector<sf::Vertex>                           springVerts;

    sf::Texture pointTexture;

  public:
    template <typename Engine>
    EntityManager(Engine&& engine_, sf::Color pointColor = sf::Color::Red,
                  sf::Color springColor = sf::Color::White)
        : engine(engine_), pointVerts(engine.points.size() * 4),
          springVerts(engine.springs.size() * 2) {
        for (const auto& p: engine.points) {
            pointColors[p.ind] = pointColor;
        }
        for (const auto& s: engine.springs) {
            springColors[s.ind] = springColor;
        }
        if (!pointTexture.loadFromFile("point.png"))
            throw std::logic_error("failed to load point texture");
        pointTexture.setSmooth(true);
    }

    template <typename P>
    void addPoint(P&& point, const sf::Color& color) {
        engine.addPoint(std::forward(point));
        pointVerts.emplace_back(sf::Vector2f{}, color, sf::Vector2f{0, 0});
        pointVerts.emplace_back(sf::Vector2f{}, color, sf::Vector2f{300, 0});
        pointVerts.emplace_back(sf::Vector2f{}, color, sf::Vector2f{300, 300});
        pointVerts.emplace_back(sf::Vector2f{}, color, sf::Vector2f{0, 300});
    }

    template <typename S>
    void addSpring(S&& spring) {
        engine.addSpring(std::forward(spring));
        springVerts.emplace_back();
        springVerts.emplace_back();
    }

    void rmvPoint(physenv::PointRef point) {
        pointVerts.resize(pointVerts.size() - 4);

        // TODO readd graph deletion
        // // remove and fix graphs
        // auto GCurr = graphs.begin();
        // auto GEnd  = graphs.end();
        // while (GCurr < GEnd) {
        //     if ((*GCurr).checkDeleteIndex(pos)) { // remove invalid point graphs
        //         std::cout << "Graph of " << GCurr->getYLabel()
        //                   << " removed due to removal of point " << static_cast<std::size_t>(pos)
        //                   << "\n";
        //         *GCurr = std::move(*(--GEnd));
        //     } else {
        //         (*GCurr).updateIndex(old, pos); // update indexs for move
        //         ++GCurr;
        //     }
        // }
        // graphs.erase(GEnd, graphs.end()); // finish the deleting of the graphs

        engine.rmvPoint(point);
    }

    void rmvSpring(physenv::SpringRef spring) {
        engine.rmvSpring(spring);
        springVerts.resize(springVerts.size() - 2); // delete

        // TODO readd graph deletion
        // auto GStart = graphs.begin();
        // auto GEnd   = graphs.end();
        // while (GStart < GEnd) {
        //     if ((*GStart).checkDeleteIndex(spring)) { // remove invalid spring graphs
        //         std::cout << "Graph of " << GStart->getYLabel()
        //                   << " removed due to removal of spring " << spring << "\n";
        //         *GStart = std::move(*(--GEnd));
        //     } else {
        //         (*GStart).updateIndex(old, spring); // update indexs for move
        //         GStart++;
        //     }
        // }
        // graphs.erase(GEnd, graphs.end()); // finish the deleting of the graphs
    }

    void setPointColor(physenv::PointRef pRef, sf::Color col) {
        pointColors[pRef] = col;
        pointColorChanged = true;
    }

    void setSpringColor(physenv::SpringRef sRef, sf::Color col) {
        springColors[sRef] = col;
        springColorChanged = true;
    }

    void drawPoints(sf::RenderWindow& window, float radius) {
        std::size_t i = 0;
        for (const auto& p: engine.points) {
            if (pointColorChanged) { // if there has been a color change upate all vertex colours
                const auto& col             = pointColors[p.ind];
                pointVerts[i * 4].color     = col;
                pointVerts[i * 4 + 1].color = col;
                pointVerts[i * 4 + 2].color = col;
                pointVerts[i * 4 + 3].color = col;
            }
            sf::Vector2f pos               = visualize(p.obj.pos);
            pointVerts[i * 4].position     = pos + sf::Vector2f{-radius, -radius};
            pointVerts[i * 4 + 1].position = pos + sf::Vector2f{radius, -radius};
            pointVerts[i * 4 + 2].position = pos + sf::Vector2f{radius, radius};
            pointVerts[i * 4 + 3].position = pos + sf::Vector2f{-radius, radius};
            ++i;
        }
        pointColorChanged = false;
        window.draw(pointVerts.data(), pointVerts.size(), sf::Quads, &pointTexture);
    }

    void drawSprings(sf::RenderWindow& window) {
        std::size_t i = 0;
        for (const auto& s: engine.springs) {
            if (springColorChanged) { // if there has been a color change upate all vertex colours
                const auto& col             = springColors[s.ind];
                pointVerts[i * 2].color     = col;
                pointVerts[i * 2 + 1].color = col;
            }
            springVerts[i * 2].position     = visualize(engine.points[s.obj.p1].pos);
            springVerts[i * 2 + 1].position = visualize(engine.points[s.obj.p2].pos);
            ++i;
        }
        window.draw(springVerts.data(), springVerts.size(), sf::Lines);
    }
};