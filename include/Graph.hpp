#pragma once

#include "RingBuffer.hpp"
#include "Vector2.hpp"
#include "implot.h"
#include <array>
#include <cstddef>
#include <cstdio>
#include <optional>
#include <string>

class EntityManager;

enum class ObjectType { Point, Spring};
enum class Property {
    Position,
    Velocity,
    Length,
    Extension,
    Force
}; // todo maybe convert to member function pointers
enum class Component { x, y, vec };

constexpr static std::array ObjectTypeLbl{"Point", "Spring"};
constexpr static std::array PropLbl{"Position", "Velocity", "Length", "Extension", "Force"};
constexpr static std::array CompLbl{"X", "Y", "Mag"};

inline std::string getTypeLbl(ObjectType type) {
    return ObjectTypeLbl[static_cast<std::size_t>(type)];
}

inline std::string getPropLbl(Property prop) { return PropLbl[static_cast<std::size_t>(prop)]; }

inline std::string getCompLbl(Component comp) { return CompLbl[static_cast<std::size_t>(comp)]; }

class Graph {
  private:
    float getValue(const EntityManager& entities);

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
    RingBuffer<Vec2F>          data;
    std::size_t                y; // TODO - move the property and type outside
    std::optional<std::size_t> y2;
    std::optional<Vec2F>       constDiff;
    ObjectType                 type;
    Property                   prop;
    Component                  comp = Component::vec;

    Graph(std::size_t y_, ObjectType type_, Property prop_, Component comp_, std::size_t buffer)
        : data(buffer), y(y_), y2(std::nullopt), type(type_), prop(prop_), comp(comp_) {}

    Graph(std::size_t y_, std::size_t y2_, ObjectType type_, Property prop_, Component comp_,
          std::size_t buffer)
        : data(buffer), y(y_), y2(y2_), type(type_), prop(prop_), comp(comp_) {}

    Graph(std::size_t y_, Vec2F constDiff_, ObjectType type_, Property prop_, Component comp_, std::size_t buffer)
        : data(buffer), y(y_), y2(std::nullopt), constDiff(constDiff_), type(type_), prop(prop_), comp(comp_) {}

    void updateIndex(ObjectType type_, std::size_t old, std::size_t updated) {
        if (type_ != type) return;
        if (y == old) y = updated;
        if (y2 && *y2 == old) y2 = updated;
    }

    bool checkDeleteIndex(ObjectType type_, std::size_t i) {
        if (type_ != type) return false;
        if (y == i) return true;
        if (y2 && *y2 == i) y2.reset();
        return false;
    }

    void add(float t, const EntityManager& entities) { data.add({t, getValue(entities)}); }

    void draw(const std::size_t& i) {
        if (ImPlot::BeginPlot(("Graph " + std::to_string(i)).c_str(), {-1, 0},
                              ImPlotFlags_NoLegend | ImPlotFlags_NoTitle)) {
            ImPlot::SetupAxis(ImAxis_X1, "Time", ImPlotAxisFlags_AutoFit);
            ImPlot::SetupAxis(ImAxis_Y1, (getPropLbl(prop) + " - " + getCompLbl(comp)).c_str(),
                              ImPlotAxisFlags_AutoFit);
            ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
            ImPlot::PlotLine("Line", &data.v[0].x, &data.v[0].y, static_cast<int>(data.v.size()),
                             ImPlotLineFlags_None, static_cast<int>(data.pos), sizeof(Vec2F));
            ImPlot::EndPlot();
        }
    }
};