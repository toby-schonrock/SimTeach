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

    void draw() {
        ImGui::Begin("Graphs");
        for (Graph& g: entities.graphs) {
            g.draw();
        }
        ImGui::End();
    }
};