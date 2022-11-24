#pragma once

#include<vector>

#include"Polygon.hpp"
#include"Point.hpp"
#include"Vector2.hpp"
#include "SFML/Graphics.hpp"

struct Spring{
    float springConst;
    float dampFact;
    float stablePoint;
    std::size_t p1;
    std::size_t p2;
};

class Sim {
    public:
    std::vector<Polygon> polys;
    std::vector<Point> points;
    std::vector<Spring> springs;
    float gravity;

    void simFrame(double deltaTime){
        // calculate spring force
        for (const Spring& spring: springs) springHandler(points[spring.p1], points[spring.p2], spring);

        // update point positions
        for (Point& point: points) point.update(deltaTime, gravity);

        // collide points with polygons
        for (const Polygon& poly: polys) {
            for (Point& point: points) {
                if (poly.isBounded(point.pos)) point.polyColHandler(poly);
            }
        }
    }

    static void springHandler(Point& p1, Point& p2,const Spring& spring) {
        Vec2   diff     = p1.pos - p2.pos.; // broken out alot "yes this is faster! really like 3x"
        double diffMag  = diff.mag();
        Vec2   diffNorm = diff / diffMag;
        double ext      = diffMag - spring.stablePoint;
        double springf  = -spring.springConst * ext; // -ke spring force and also if a diagonal increase
                                              // spring constant for stability // test
        double dampf = diffNorm.dot(p2.vel - p1.vel) * spring.dampFact; // damping force
        Vec2   force = (springf + dampf) * diffNorm;
        p1.f += force; // equal and opposite reaction
        p2.f -= force;
    }
};