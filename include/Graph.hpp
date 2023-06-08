#pragma once

#include "Fundamentals/RingBuffer.hpp"
#include "Fundamentals/Vector2.hpp"
#include "implot.h"
#include <array>
#include <cstddef>
#include <cstdio>
#include <optional>
#include <string>

class EntityManager;

enum class ObjectType { Point, Spring };
enum class Property { Position, Velocity, Length, Extension, Force };
enum class Component { x, y, vec };
enum class DiffState { None, Index, Const };

constexpr static std::array ObjTypeLbl{"Point", "Spring"};
constexpr static std::array PropLbl{"Position", "Velocity", "Length", "Extension", "Force"};
constexpr static std::array CompLbl{"X", "Y", "Mag"};
constexpr static std::array DiffStatLbl{"None", "Obj", "Const"};

inline std::string getTypeLbl(ObjectType type) {
    return ObjTypeLbl[static_cast<std::size_t>(type)];
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
    RingBuffer<float> data;
    std::size_t       ref;
    std::size_t       ref2;
    Vec2F             constDiff;
    ObjectType        type;
    Property          prop;
    Component         comp = Component::vec;
    DiffState         diff;

    // three constructors for all diff types
    // no diff
    Graph(std::size_t ref_, ObjectType type_, Property prop_, Component comp_, std::size_t buffer)
        : data(buffer), ref(ref_), type(type_), prop(prop_), comp(comp_), diff(DiffState::None) {}

    // index diff
    Graph(std::size_t ref_, std::size_t ref2_, ObjectType type_, Property prop_, Component comp_,
          std::size_t buffer)
        : data(buffer), ref(ref_), ref2(ref2_), type(type_), prop(prop_), comp(comp_),
          diff(DiffState::Index) {}

    // const diff
    Graph(std::size_t ref_, Vec2F constDiff_, ObjectType type_, Property prop_, Component comp_,
          std::size_t buffer)
        : data(buffer), ref(ref_), constDiff(constDiff_), type(type_), prop(prop_), comp(comp_),
          diff(DiffState::Const) {}

    // universal constructor will figure it out
    Graph(DiffState diff_, std::size_t ref_, std::size_t ref2_, Vec2F constDiff_, ObjectType type_,
          Property prop_, Component comp_, std::size_t buffer)
        : data(buffer), ref(ref_), ref2(ref2_), constDiff(constDiff_), type(type_), prop(prop_),
          comp(comp_), diff(diff_) {}

    void updateIndex(ObjectType type_, std::size_t old, std::size_t updated) {
        if (type_ != type) return;
        if (ref == old) ref = updated;
        if (ref2 == old) ref2 = updated;
    }

    bool checkDeleteIndex(ObjectType type_, std::size_t i) {
        if (type_ != type) return false;
        if (ref == i) return true;
        if (ref2 == i) diff = DiffState::None;
        return false;
    }

    void add(const EntityManager& entities) { data.add(getValue(entities)); }

    void draw(const std::size_t& i, const RingBuffer<float>& yvalues) {
        if (ImPlot::BeginPlot(("Graph " + std::to_string(i)).c_str(), {-1, 0},
                              ImPlotFlags_NoLegend | ImPlotFlags_NoTitle)) {
            ImPlot::SetupAxis(ImAxis_X1, "Time", ImPlotAxisFlags_AutoFit);
            ImPlot::SetupAxis(ImAxis_Y1, getYLabel().c_str(), ImPlotAxisFlags_AutoFit);
            ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
            ImPlot::PlotLine("Line", &yvalues.v[0], &data.v[0], static_cast<int>(data.v.size()),
                             ImPlotLineFlags_None, static_cast<int>(data.pos));
            ImPlot::EndPlot();
        }
    }

    std::string getYLabel() const {
        return getTypeLbl(type) + "(" +
               (diff == DiffState::Index ? std::to_string(ref) + "-" + std::to_string(ref2)
                                         : std::to_string(ref)) +
               ")." + getPropLbl(prop) + "." + getCompLbl(comp);
    }
};