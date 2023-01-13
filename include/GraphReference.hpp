#pragma once

#include "Spring.hpp"
#include "Vector2.hpp"
#include <array>
#include <functional>

class EntityManager;

enum class ObjectType { Point, Spring, Const };
enum class Property { Position, Velocity, Extension, Force };
enum class Component { Position, Velocity, Extension, Force };f

constexpr static std::array ObjectTypeLbl{"Point", "Spring", "Const"};
constexpr static std::array PropLbl{"Position", "Velocity", "Extension", "Force"};

inline std::string_view getTypeLbl(ObjectType type) {
    return ObjectTypeLbl[static_cast<std::size_t>(type)];
}

inline std::string_view getPropLbl(Property prop) {
    return PropLbl[static_cast<std::size_t>(prop)];
}

class DataReference {
  public:
    std::size_t index;
    ObjectType  type;
    Property    prop;
    Vec2F       value;

    DataReference(const std::size_t& index_, ObjectType type_, Property prop_, Vec2F value_ = {})
        : index(index_), type(type_), prop(prop_), value(value_) {}

    Vec2F getValue(const EntityManager& entities);
};