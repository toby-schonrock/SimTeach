#include "Graph.hpp"
#include "EntityManager.hpp"
#include "Fundamentals/Vector2.hpp"
#include <iostream>
#include <optional>

// retrieve value from entities
float Graph::getValue(const EntityManager& entities) {
    Vec2 value;
    Vec2 value2;
    switch (type) {
    case ObjectType::Point:
        switch (prop) {
        case Property::Position:
            value = entities.points[ref].pos;
            if (diff == DiffState::Index) value2 = entities.points[ref2].pos;
            break;
        case Property::Velocity:
            value = entities.points[ref].vel;
            if (diff == DiffState::Index) value2 = entities.points[ref2].vel;
            break;
        default:
            throw std::logic_error("Incorrect graph point enums"); // bad setup of graph
        }
        break;
    case ObjectType::Spring: {
        const Spring& s = entities.springs[ref];
        if (diff == DiffState::Index) {
            const Spring& s2 = entities.springs[ref2];
            switch (prop) {
            case Property::Length:
                value2 = entities.points[s2.p1].pos - entities.points[s2.p2].pos;
                break;
            case Property::Extension:
                value2 = entities.points[s2.p1].pos - entities.points[s2.p2].pos;
                value2 = value2 - value2.norm() * s2.naturalLength;
                break;
            case Property::Force:
                value2 = s2.forceCalc(entities.points[s2.p1], entities.points[s2.p2]);
                break;
            default:
                throw std::logic_error("Incorrect graph spring enums"); // bad setup of graph
            }
        }
        switch (prop) {
        case Property::Length:
            value = entities.points[s.p1].pos - entities.points[s.p2].pos;
            break;
        case Property::Extension:
            value = entities.points[s.p1].pos - entities.points[s.p2].pos; // gap
            value = value - value.norm() * s.naturalLength;
            break;
        case Property::Force:
            value = s.forceCalc(entities.points[s.p1], entities.points[s.p2]);
            break;
        default:
            throw std::logic_error("Incorrect graph spring enums"); // bad setup of graph
        }
    }
    }
    if (diff == DiffState::Const) value2 = Vec2(constDiff);
    return getComponent(Vec2F(value - value2));
}