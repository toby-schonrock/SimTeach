#pragma once

#include "EntityManager.hpp"
#include "Graph.hpp"
#include <cstddef>

class GraphManager {
    EntityManager& entities;

    public:
    GraphManager(EntityManager& entities_) : entities(entities_) {}

    void updateDraw(float t) {
        ImGui::Begin("Graphs");
        for (std::size_t i = 0; i != entities.graphs.size(); ++i) {
            entities.graphs[i].add(t, entities);
            entities.graphs[i].draw(i);
        }
        ImGui::End();
    }

    void reset(std::size_t bufferSize) {
        for (Graph& g: entities.graphs) {
            g.data = RingBuffer<Vec2F>(bufferSize);
        }
    }

    bool valid() { // TODO remove me see graph.cpp
        for (Graph& g: entities.graphs) {
            if (!g.isValid(entities)) return false; 
        }
        return true;
    }
};