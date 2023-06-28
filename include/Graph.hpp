#pragma once

#include "Fundamentals/RingBuffer.hpp"
#include "Fundamentals/Vector2.hpp"
#include "Point.hpp"
#include "Spring.hpp"
#include "implot.h"
#include <array>
#include <cstddef>
#include <cstdio>
#include <optional>
#include <string>
#include <type_traits>

class EntityManager;

enum class ObjectType { Point, Spring };
enum class Property { Position, Velocity, Length, Extension, Force };
enum class Component { x, y, vec };
enum class DiffState { None, Index, Const };
enum class GraphId : std::size_t;

constexpr static std::array ObjTypeLbl{"Point", "Spring"};
constexpr static std::array PropLbl{"Position", "Velocity", "Length", "Extension", "Force"};
constexpr static std::array CompLbl{"X", "Y", "Mag"};
constexpr static std::array DiffStatLbl{"None", "Obj", "Const"};

inline std::string getTypeLbl(ObjectType type) {
    return ObjTypeLbl[static_cast<std::size_t>(type)];
}

inline std::string getPropLbl(Property prop) { return PropLbl[static_cast<std::size_t>(prop)]; }

inline std::string getCompLbl(Component comp) { return CompLbl[static_cast<std::size_t>(comp)]; }

template <typename T>
concept Index = std::is_same_v<T, PointId> || std::is_same_v<T, SpringId>;

class Graph {
  private:
    union Inflex {
        PointId     p;
        SpringId    s;
        std::size_t getUnderlying(ObjectType indexType) const {
            return indexType == ObjectType::Point ? static_cast<std::size_t>(p)
                                                  : static_cast<std::size_t>(s);
        }
    };

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
    Inflex            ref;
    Inflex            ref2;
    Vec2F             constDiff;
    ObjectType        type;
    Property          prop;
    Component         comp = Component::vec;
    DiffState         diff;

    // three constructors for all diff types
    // no diff
    template <Index IndexType>
    Graph(IndexType ref_, Property prop_, Component comp_, std::size_t buffer)
        : data(buffer), ref(ref_), prop(prop_), comp(comp_), diff(DiffState::None) {
        if constexpr (std::is_same_v<IndexType, PointId>)
            type = ObjectType::Point;
        else
            type = ObjectType::Spring;
    }

    // index diff
    template <Index IndexType>
    Graph(IndexType ref_, IndexType ref2_, Property prop_, Component comp_, std::size_t buffer)
        : data(buffer), ref(ref_), ref2(ref2_), prop(prop_), comp(comp_), diff(DiffState::Index) {
        if constexpr (std::is_same_v<IndexType, PointId>)
            type = ObjectType::Point;
        else
            type = ObjectType::Spring;
    }

    // const diff
    template <Index IndexType>
    Graph(IndexType ref_, Vec2F constDiff_, Property prop_, Component comp_, std::size_t buffer)
        : data(buffer), ref(ref_), constDiff(constDiff_), prop(prop_), comp(comp_),
          diff(DiffState::Const) {
        if constexpr (std::is_same_v<IndexType, PointId>)
            type = ObjectType::Point;
        else
            type = ObjectType::Spring;
    }

    void updateIndex(PointId old, PointId updated) {
        if (type != ObjectType::Point) return;
        if (ref.p == old) ref.p = updated;
        if (ref2.p == old) ref2.p = updated;
    }

    void updateIndex(SpringId old, SpringId updated) {
        if (type != ObjectType::Spring) return;
        if (ref.s == old) ref.s = updated;
        if (ref2.s == old) ref2.s = updated;
    }

    bool checkDeleteIndex(PointId id) {
        if (type != ObjectType::Point) return false;
        if (ref.p == id) return true;
        if (ref2.p == id) diff = DiffState::None;
        return false;
    }

    bool checkDeleteIndex(SpringId id) {
        if (type != ObjectType::Spring) return false;
        if (ref.s == id) return true;
        if (ref2.s == id) diff = DiffState::None;
        return false;
    }

    void add(const EntityManager& entities) { data.add(getValue(entities)); }

    void draw(GraphId i, const RingBuffer<float>& yvalues) {
        if (ImPlot::BeginPlot(("Graph " + std::to_string(static_cast<std::size_t>(i))).c_str(), {-1, 0},
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
               (diff == DiffState::Index ? std::to_string(ref.getUnderlying(type)) + "-" +
                                               std::to_string(ref2.getUnderlying(type))
                                         : std::to_string(ref.getUnderlying(type))) +
               ")." + getPropLbl(prop) + "." + getCompLbl(comp);
    }
};