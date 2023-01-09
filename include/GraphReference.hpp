#pragma once

#include "Spring.hpp"
#include "Vector2.hpp"
#include <array>
#include <functional>

class EntityManager;

enum class ObjectType { Point, Spring, Const };
enum class PointProp { Position, Velocity, Acceleration };
enum class SpringProp { Extension, Force };

using namespace std::string_view_literals;
constexpr static std::array ObjectTypeLbl{"Point"sv, "Spring"sv, "Const"sv};
constexpr static std::array PointPropLbl{"Position"sv, "Velocity"sv, "Acceleration"sv};
constexpr static std::array SpringPropLbl{"Extension"sv, "Force"sv};

inline std::string_view getTypeLbl(ObjectType type) {
    return ObjectTypeLbl[static_cast<std::size_t>(type)];
}

inline std::string_view getPropLbl(PointProp prop) {
    return PointPropLbl[static_cast<std::size_t>(prop)];
}

inline std::string_view getPropLbl(SpringProp prop) {
    return SpringPropLbl[static_cast<std::size_t>(prop)];
}

class DataReference {
  public:
    std::size_t index;

    DataReference(const std::size_t& index_) : index(index_) {}

    virtual ~DataReference()                  = default;
    DataReference(const DataReference& other) = delete;
    DataReference& operator=(const DataReference& other) = delete;

    virtual Vec2F      getValue(const EntityManager& entities) = 0;
    virtual ObjectType getType()                               = 0;
};

class PointReference : public DataReference {
  private:
    std::function<Vec2F(const Point&)> Retrieve;

  public:
    PointProp prop;

    PointReference(const std::size_t& index_, PointProp prop_,
                   std::function<Vec2F(const Point&)> Retrieve_)
        : DataReference(index_), Retrieve(std::move(Retrieve_)), prop(prop_) {}

    Vec2F getValue(const EntityManager& entities) override;

    ObjectType getType() override { return ObjectType::Point; }
};

class SpringReference : public DataReference {
  private:
    std::function<Vec2F(const Spring&)> Retrieve;

  public:
    SpringProp prop;

    SpringReference(const std::size_t& index_, SpringProp prop_,
                    std::function<Vec2F(const Spring&)> Retrieve_)
        : DataReference(index_), Retrieve(std::move(Retrieve_)), prop(prop_) {}

    Vec2F getValue(const EntityManager& entities) override;

    ObjectType getType() override { return ObjectType::Spring; }
};

class ConstReference : public DataReference {
  private:
    Vec2F value;

  public:
    ConstReference(const std::size_t& index_, const Vec2F& value_)
        : DataReference(index_), value(value_) {}

    Vec2F getValue([[maybe_unused]] const EntityManager& entities) override { return value; }

    ObjectType getType() override { return ObjectType::Const; }
};