#pragma once

#include "GraphReference.hpp"
#include "RingBuffer.hpp"
#include "imgui_internal.h"
#include "implot.h"
#include <optional>
#include <string>
#include <vector>

class Graph {
  private:
    float getValue(const EntityManager& entities) {
        if (!y) throw std::logic_error("Cannot get value on an empty graph");
        if (!y2) return y->getValue(entities).mag();
        return (y->getValue(entities) - y2->getValue(entities)).mag();
    }

  public:
    RingBuffer<Vec2F>            data{10000};
    std::optional<DataReference> y;
    std::optional<DataReference> y2;

    Graph(const DataReference& y_) : y(y_), y2(std::nullopt) {}

    Graph(const DataReference& y_, const DataReference& y2_) : y(y_), y2(y2_) {}

    void updateIndex(ObjectType type, std::size_t old, std::size_t updated) {
        if (y && y->type == type && y->index == old) y->index = updated;
        if (y2 && y2->type == type && y2->index == old) y2->index = updated;
    }

    void checkDeleteIndex(ObjectType type, std::size_t i) {
        if (y && y->type == type && y->index == i) y.reset();
        if (y2 && y2->type == type && y2->index == i) y2.reset();
    }

    void add(float t, const EntityManager& entities) { data.add({t, getValue(entities)}); }

    void draw() {
        if (ImPlot::BeginPlot("Graph")) {
            ImPlot::SetupAxis(ImAxis_X1, "Time", ImPlotAxisFlags_AutoFit);
            ImPlot::SetupAxis(ImAxis_Y1, getPropLbl(y->prop).begin(), ImPlotAxisFlags_AutoFit);
            ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
            ImPlot::PlotLine("test", &data.v[0].x, &data.v[0].y, static_cast<int>(data.v.size()),
                             ImPlotLineFlags_None, static_cast<int>(data.pos), sizeof(Vec2F));
            ImPlot::EndPlot();
        }
    }
};