#pragma once

#include "EntityManager.hpp"
#include "Graph.hpp"

class GraphManager {
    EntityManager& entities;

    public:
    GraphManager(EntityManager& entities_) : entities(entities_) {}

    void updateDraw(float t) {
        ImGui::Begin("Graphs");
        for (Graph& g: entities.graphs) {
            g.add(t, entities);
            g.draw();
        }
        ImGui::End();
    }

    void reset() {
        for (Graph& g: entities.graphs) {
            g.data.reset();
        }
    }
};