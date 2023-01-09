#pragma once

#include "Spring.hpp"
#include "Vector2.hpp"
#include <array>
#include <functional>

class EntityManager;

enum class ObjectType { Point, Spring, Const };
enum class Property { Position, Velocity, Extension, Force };

using namespace std::string_view_literals;
constexpr static std::array ObjectTypeLbl{"Point"sv, "Spring"sv, "Const"sv};
constexpr static std::array PropLbl{"Position"sv, "Velocity"sv, "Extension"sv, "Force"sv};

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