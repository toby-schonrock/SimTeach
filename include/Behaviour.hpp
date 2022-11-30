#pragma once

#include "SFML/Graphics.hpp"
#include "SFML/System/Vector2.hpp"
#include "SFML/Window.hpp"
#include "Sim.hpp"
#include <cstddef>
#include <iostream>
#include <optional>

sf::Vector2f visualize(const Vec2& v);
Vec2         unvisualize(const sf::Vector2f& v);

class Behaviour {
  public:
    Behaviour() = default;
    virtual void frame(Sim& sim, const sf::RenderWindow& window, const sf::Vector2i& mousePos) = 0;
    virtual void event(Sim& sim, sf::Event event)                                              = 0;
    Behaviour(const Behaviour& other) = delete;
    Behaviour& operator=(const Behaviour& other) = delete;
};

class SoftBody : public Behaviour {
  private:
    static inline const sf::Color selectedColour = sf::Color::Blue;
    static constexpr double       sliceRange     = 0.20;
    static constexpr double       deleteRange    = 1;
    std::optional<std::size_t>    closestPoint   = std::nullopt;
    double                        closestDist    = 0;

  public:
    SoftBody() = default;

    void frame(Sim& sim, const sf::RenderWindow& window, const sf::Vector2i& mousePos) override {
        // if sim has no points nothing to do (may change)
        if (sim.points.size() == 0) return;

        if (closestPoint)
            sim.points[*closestPoint].shape.setFillColor(
                sim.color); // reset last closest point color as it may not be closest anymore

        // determine new closest point
        Vec2 localMousePos = unvisualize(window.mapPixelToCoords(mousePos));
        auto close    = sim.findClosestPoint(localMousePos);
        closestPoint  = close.first;
        closestDist   = close.second;
        // color close point for selection
        if (closestDist < deleteRange) {
            sim.points[*closestPoint].shape.setFillColor(selectedColour);
        } else {
            closestPoint.reset();
        }

        // if the current closest is close enough delete it. then search for all others in range and
        // delete them
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && closestDist < sliceRange) {
            sim.removePoint(*closestPoint);
            closestPoint.reset();
            if (sim.points.size() == 0) return;
            auto pointsInRange = sim.findPointsInRange(
                localMousePos, sliceRange); // this is in reverse order so I can delete without errors
            for (std::size_t& p: pointsInRange) {
                sim.removePoint(p);
            }
        };
    }

    void event(Sim& sim, sf::Event event) override {
        if (closestPoint && event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left) {
            if (closestDist < deleteRange) {
                sim.removePoint(*closestPoint);
                closestPoint.reset();
            }
        }
    }
};