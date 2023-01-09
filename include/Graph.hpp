#pragma once

#include "GraphReference.hpp"
#include "RingBuffer.hpp"
#include <memory>
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
    RingBuffer<Vec2F>              data{500};
    std::unique_ptr<DataReference> y;
    std::unique_ptr<DataReference> y2;

    Graph(PointProp prop, std::size_t index) {
        y = std::make_unique<PointReference>(index, prop,
                                             [](const Point& p) { return Vec2F(p.pos); });
    }

    void updateIndex(ObjectType type, std::size_t old, std::size_t updated) {
        if (y && y->getType() == type && y->index == old) y->index = updated;
        if (y2 && y2->getType() == type && y2->index == old) y2->index = updated;
    }

    void checkDeleteIndex(ObjectType type, std::size_t i) {
        if (y && y->getType() == type && y->index == i) y.reset(nullptr);
        if (y2 && y2->getType() == type && y2->index == i) y2.reset(nullptr);
    }
};