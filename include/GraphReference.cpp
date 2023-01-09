#include "GraphReference.hpp"
#include "EntityManager.hpp"

Vec2F PointReference::getValue(const EntityManager& entities) {
    return Retrieve(entities.points[index]);
}

Vec2F SpringReference::getValue(const EntityManager& entities) {
    return Retrieve(entities.springs[index]);
}