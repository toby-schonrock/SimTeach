#pragma once

#include "Vector2.hpp"
#include "Point.hpp"

struct Spring {
    double      springConst;
    double      dampFact;
    double      stablePoint;
    std::size_t p1;
    std::size_t p2;

    void springHandler(Point& point1, Point& point2) const {
        Vec2 diff = point1.pos - point2.pos; // broken out alot "yes this is faster! really like 3x"
        double diffMag  = diff.mag();
        Vec2   diffNorm = diff / diffMag;
        double ext      = diffMag - stablePoint;
        double springf  = -springConst * ext; // -ke spring force and also if a diagonal increase
                                              // spring constant for stability // test
        double dampf = diffNorm.dot(point2.vel - point1.vel) * dampFact; // damping force
        Vec2   force = (springf + dampf) * diffNorm;
        point1.f += force; // equal and opposite reaction
        point2.f -= force;
    }
};
