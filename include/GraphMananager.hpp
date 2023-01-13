#pragma once

#include "EntityManager.hpp"
#include "Graph.hpp"
#include <cstddef>

class GraphManager {
  private:
    EntityManager& entities;

  public:
    std::size_t graphBuffer = 5000;
    GraphManager(EntityManager& entities_) : entities(entities_) {}

    void updateDraw(float t) {
        ImGui::Begin("Graphs");
        for (std::size_t i = 0; i != entities.graphs.size(); ++i) {
            entities.graphs[i].add(t, entities);
            entities.graphs[i].draw(i);
        }
        ImGui::End();
    }

    void reset() {
        for (Graph& g: entities.graphs) {
            g.data = RingBuffer<Vec2F>(graphBuffer);
        }
    }

    void makeNew() {
        entities.graphs.emplace_back(graphBuffer);
    }

    bool valid() { // TODO remove me see graph.cpp
        for (Graph& g: entities.graphs) {
            if (!g.isValid(entities)) return false;
        }
        return true;
    }
};