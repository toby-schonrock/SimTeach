#include "Graph.hpp"
#include "EntityManager.hpp"
#include "Vector2.hpp"
#include <iostream>
#include <optional>

float Graph::getValue(const EntityManager& entities) {
    Vec2                value;
    std::optional<Vec2> value2;
    switch (type) {
    case ObjectType::Point:
        switch (prop) {
        case Property::Position:
            value = entities.points[y].pos;
            if (y2) value2 = entities.points[*y2].pos;
            break;
        case Property::Velocity:
            value = entities.points[y].vel;
            if (y2) value2 = entities.points[*y2].vel;
            break;
        default:
            throw std::logic_error("Incorrect graph point enums"); // bad setup of graph
        }
        break;
    case ObjectType::Spring: {
        const Spring& s = entities.springs[y];
        if (y2) {
            const Spring& s2 = entities.springs[*y2];
            switch (prop) {
            case Property::Length:
                value  = entities.points[s.p1].pos - entities.points[s.p2].pos;
                value2 = entities.points[s2.p1].pos - entities.points[s2.p2].pos;
                break;
            case Property::Extension:
                value = {0, 0};
                break;
                // TODO implement extension
            case Property::Force:
                value  = s.forceCalc(entities.points[s.p1], entities.points[s.p2]);
                value2 = s2.forceCalc(entities.points[s2.p1], entities.points[s2.p2]);
                break;
            default:
                throw std::logic_error("Incorrect graph spring enums"); // bad setup of graph
            }
        }
        switch (prop) {
        case Property::Length:
            value  = entities.points[s.p1].pos - entities.points[s.p2].pos;
            break;
        case Property::Extension:
            value = {0, 0};
            break;
            // TODO implement extension
        case Property::Force:
            value  = s.forceCalc(entities.points[s.p1], entities.points[s.p2]);
            break;
        default:
            throw std::logic_error("Incorrect graph spring enums"); // bad setup of graph
        }
    }
    }
    if (constDiff) value2 = Vec2(*constDiff);
    if (value2) return getComponent(Vec2F(value - *value2));
    return getComponent(Vec2F(value));
}