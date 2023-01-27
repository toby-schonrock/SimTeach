#pragma once

#include "DataReference.hpp"
#include "RingBuffer.hpp"
#include "implot.h"
#include <cstddef>
#include <cstdio>
#include <optional>
#include <string>
#include <vector>

class Graph {
  private:
    float getValue(const EntityManager& entities) {
        if (!y2) return getComponent(y.getValue(entities));
        return getComponent(y.getValue(entities) - y2->getValue(entities));
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
    RingBuffer<Vec2F>            data;
    DataReference y;
    std::optional<DataReference> y2;
    Component                    comp = Component::vec;

    Graph(const DataReference& y_, Component comp_, std::size_t buffer)
        : data(buffer), y(y_), y2(std::nullopt), comp(comp_) {}

    Graph(const DataReference& y_, const DataReference& y2_, Component comp_, std::size_t buffer)
        : data(buffer), y(y_), y2(y2_), comp(comp_) {}

    void updateIndex(ObjectType type, std::size_t old, std::size_t updated) {
        if (y.type == type && y.index == old) y.index = updated;
        if (y2 && y2->type == type && y2->index == old) y2->index = updated;
    }

    bool checkDeleteIndex(ObjectType type, std::size_t i) {
        if (y.type == type && y.index == i) return true;
        if (y2 && y2->type == type && y2->index == i) y2.reset();
        return false;
    }

    void add(float t, const EntityManager& entities) { data.add({t, getValue(entities)}); }

    void draw(const std::size_t& i) {
        if (ImPlot::BeginPlot(("Graph " + std::to_string(i)).c_str(), {-1, 0},
                              ImPlotFlags_NoLegend | ImPlotFlags_NoTitle)) {
            ImPlot::SetupAxis(ImAxis_X1, "Time", ImPlotAxisFlags_AutoFit);
            ImPlot::SetupAxis(ImAxis_Y1, (getPropLbl(y.prop) + " - " + getCompLbl(comp)).c_str(),
                              ImPlotAxisFlags_AutoFit);
            ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
            ImPlot::PlotLine("Line", &data.v[0].x, &data.v[0].y, static_cast<int>(data.v.size()),
                             ImPlotLineFlags_None, static_cast<int>(data.pos), sizeof(Vec2F));
            ImPlot::EndPlot();
        }
    }
};