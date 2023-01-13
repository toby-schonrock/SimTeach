#pragma once

#include "GraphReference.hpp"
#include "RingBuffer.hpp"
#include "imgui_internal.h"
#include "implot.h"
#include <cstddef>
#include <cstdio>
#include <optional>
#include <string>
#include <vector>

class Graph {
  private:
    float getValue(const EntityManager& entities) {
        if (!y) throw std::logic_error("Cannot get value on an empty graph");
        if (!y2) return getComponent(y->getValue(entities));
        return getComponent(y->getValue(entities) - y2->getValue(entities));
    }

    float getComponent(Vec2F value) {
        switch (comp) {
        case Component::vec:
            return value.mag();
        case Component::x:
            return value.x;
        case Component::y:
            return value.y;
        }
        return 0; // unreachable
    }

  public:
    RingBuffer<Vec2F>            data{10000};
    std::optional<DataReference> y;
    std::optional<DataReference> y2;
    Component                    comp = Component::vec;

    Graph() {}
    
    Graph(const DataReference& y_, Component comp_) : y(y_), y2(std::nullopt), comp(comp_) {}

    Graph(const DataReference& y_, const DataReference& y2_, Component comp_)
        : y(y_), y2(y2_), comp(comp_) {}

    void updateIndex(ObjectType type, std::size_t old, std::size_t updated) {
        if (y && y->type == type && y->index == old) y->index = updated;
        if (y2 && y2->type == type && y2->index == old) y2->index = updated;
    }

    bool isValid(const EntityManager& entities);

    void checkDeleteIndex(ObjectType type, std::size_t i) {
        if (y && y->type == type && y->index == i) y.reset();
        if (y2 && y2->type == type && y2->index == i) y2.reset();
    }

    void add(float t, const EntityManager& entities) { data.add({t, getValue(entities)}); }

    void draw(const std::size_t& i) {
        if (ImPlot::BeginPlot(("Graph " + std::to_string(i)).data(), {-1, 0}, ImPlotFlags_NoLegend | ImPlotFlags_NoTitle)) {
            ImPlot::SetupAxis(ImAxis_X1, "Time", ImPlotAxisFlags_AutoFit);
            ImPlot::SetupAxis(ImAxis_Y1, (getPropLbl(y->prop) + " - " + getCompLbl(comp)).data(), ImPlotAxisFlags_AutoFit);
            ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
            ImPlot::PlotLine("Line", &data.v[0].x, &data.v[0].y, static_cast<int>(data.v.size()),
                             ImPlotLineFlags_None, static_cast<int>(data.pos), sizeof(Vec2F));
            ImPlot::EndPlot();
        }
    }
};