#include "Graph.hpp"
#include "EntityManager.hpp"
#include "Fundamentals/Vector2.hpp"
#include <cstddef>
#include <iostream>
#include <optional>
#include <string>

// retrieve value from entities
float Graph::getValue(const EntityManager& entities) {
    Vec2 value;
    Vec2 value2;
    switch (type) {
    case ObjectType::Point:
        switch (prop) {
        case Property::Position:
            value = entities.points[ref.getUnderlying(type)].pos;
            if (diff == DiffState::Index) value2 = entities.points[ref2.getUnderlying(type)].pos;
            break;
        case Property::Velocity:
            value = entities.points[ref.getUnderlying(type)].vel;
            if (diff == DiffState::Index) value2 = entities.points[ref2.getUnderlying(type)].vel;
            break;
        default:
            throw std::logic_error("Incorrect graph point enums"); // bad setup of graph
        }
        break;
    case ObjectType::Spring: {
        const Spring& s = entities.springs[ref.getUnderlying(type)];
        if (diff == DiffState::Index) {
            const Spring& s2 = entities.springs[ref2.getUnderlying(type)];
            switch (prop) {
            case Property::Length:
                value2 = entities.points[static_cast<std::size_t>(s2.p1)].pos - entities.points[static_cast<std::size_t>(s2.p2)].pos;
                break;
            case Property::Extension:
                value2 = entities.points[static_cast<std::size_t>(s2.p1)].pos - entities.points[static_cast<std::size_t>(s2.p2)].pos;
                value2 = value2 - value2.norm() * s2.naturalLength;
                break;
            case Property::Force:
                value2 = s2.forceCalc(entities.points[static_cast<std::size_t>(s2.p1)], entities.points[static_cast<std::size_t>(s2.p2)]);
                break;
            default:
                throw std::logic_error("Incorrect graph spring enums"); // bad setup of graph
            }
        }
        switch (prop) {
        case Property::Length:
            value = entities.points[static_cast<std::size_t>(s.p1)].pos - entities.points[static_cast<std::size_t>(s.p2)].pos;
            break;
        case Property::Extension:
            value = entities.points[static_cast<std::size_t>(s.p1)].pos - entities.points[static_cast<std::size_t>(s.p2)].pos; // gap
            value = value - value.norm() * s.naturalLength;
            break;
        case Property::Force:
            value = s.forceCalc(entities.points[static_cast<std::size_t>(s.p1)], entities.points[static_cast<std::size_t>(s.p2)]);
            break;
        default:
            throw std::logic_error("Incorrect graph spring enums"); // bad setup of graph
        }
    }
    }
    if (diff == DiffState::Const) value2 = Vec2(constDiff);
    return getComponent(Vec2F(value - value2));
}