#include "Graph.hpp"
#include "EntityManager.hpp" 

bool Graph::isValid(const EntityManager& entities) { // TODO remove me if not needed
    if (!y) { // no reference
        return false;
    } else { // if y1
        if ((y->type == ObjectType::Point && y->index >= entities.points.size()) ||
            (y->type == ObjectType::Spring &&
             y->index >= entities.springs.size())) { // y1 out of range
            return false;
        }
    }
    if (y2) {
        if ((y2->type == ObjectType::Point && y2->index >= entities.points.size()) ||
            (y2->type == ObjectType::Spring &&
             y2->index >= entities.springs.size())) { // y2 out of range
            return false;
        }
    }
    return true;
}