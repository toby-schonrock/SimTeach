#pragma once

#include "Point.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"
#include "Sim.hpp"
#include "Vector2.hpp"
#include "imgui.h"
#include <cstddef>
#include <exception>
#include <iostream>
#include <optional>

sf::Vector2f visualize(const Vec2& v);
Vec2         unvisualize(const sf::Vector2f& v);

inline bool ImGui_DragDouble(const char* label, double* v, float v_speed = 1.0f, double v_min = 0,
                             double v_max = 0, const char* format = 0, ImGuiSliderFlags flags = 0) {
    return ImGui::DragScalar(label, ImGuiDataType_Double, v, v_speed, &v_min, &v_max, format,
                             flags);
}

class Tool {
  public:
    const sf::RenderWindow& window;
    Tool(const sf::RenderWindow& window_) : window(window_) {}
    virtual void frame(Sim& sim, const sf::Vector2i& mousePixPos)   = 0;
    virtual void event(Sim& sim, const sf::Event& event) = 0;
    Tool(const Tool& other)                              = delete;
    Tool& operator=(const Tool& other) = delete;
};

class T_Slice : public Tool {
  private:
    static inline const sf::Color selectedColour = sf::Color::Blue;
    static constexpr double       sliceRange     = 0.20;
    static constexpr double       deleteRange    = 1;
    std::optional<std::size_t>    closestPoint   = std::nullopt;
    double                        closestDist    = 0;

  public:
    explicit T_Slice(const sf::RenderWindow& window_) : Tool(window_) {}

    void frame(Sim& sim, const sf::Vector2i& mousePixPos) override {
        // if sim has no points nothing to do (may change)
        if (sim.points.size() == 0) return;

        if (closestPoint)
            sim.points[*closestPoint].shape.setFillColor(
                sim.color); // reset last closest point color as it may not be closest anymore

        Vec2 mousePos = unvisualize(window.mapPixelToCoords(mousePixPos));
        // determine new closest point
        auto close   = sim.findClosestPoint(mousePos);
        closestPoint = close.first;
        closestDist  = close.second;
        // color close point for selection
        if (closestDist < deleteRange) {
            sim.points[*closestPoint].shape.setFillColor(selectedColour);
        } else {
            closestPoint.reset();
        }

        // if the current closest is close enough delete it. then search for all others in range and
        // delete them
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && closestPoint &&
            closestDist < sliceRange) {
            sim.removePoint(*closestPoint);
            closestPoint.reset();
            if (sim.points.size() == 0) return;
            auto pointsInRange = sim.findPointsInRange(
                mousePos, sliceRange); // this is in reverse order so I can delete without errors
            for (std::size_t& p: pointsInRange) {
                sim.removePoint(p);
            }
        };
    }

    void event(Sim& sim, const sf::Event& event) override {
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left && closestPoint) {
                sim.removePoint(*closestPoint);
                closestPoint.reset();
            }
        }
    }
};

class T_Points : public Tool {
  private:
    static inline const sf::Color selectedColour  = sf::Color::Magenta;
    static inline const sf::Color hoverColour     = sf::Color::Blue;
    std::optional<std::size_t>    closestPoint    = std::nullopt;
    std::optional<std::size_t>    currentSelected = std::nullopt;
    static constexpr double       toolRange       = 1;
    double                        closestDist     = 0;
    bool dragPos = false;

    void removePoint(Sim& sim, const std::size_t& pos){
        sim.removePoint(pos);
        if (*currentSelected == pos) currentSelected.reset();
        if (*closestPoint == pos) closestPoint.reset();
    }

  public:
    explicit T_Points(const sf::RenderWindow& window_) : Tool(window_) {}

    void frame(Sim& sim, const sf::Vector2i& mousePixPos) override {
        if (sim.points.size() == 0) return;

        if (currentSelected) { // edit menu
            IMedit(sim, mousePixPos);
        } else {
            if (closestPoint)
                sim.points[*closestPoint].shape.setFillColor(
                    sim.color); // reset last closest point color as it may not be closest anymore

            // determine new closest point
            Vec2 mousePos = unvisualize(window.mapPixelToCoords(mousePixPos));
            auto close   = sim.findClosestPoint(mousePos); // NOLINT yes I did thanks :)
            closestPoint = close.first;
            closestDist  = close.second;
            // color close point for selection
            if (closestDist < toolRange) {
                sim.points[*closestPoint].shape.setFillColor(hoverColour);
            } else {
                closestPoint.reset();
            }
        }
    }

    void event(Sim& sim, const sf::Event& event) override {
        if (event.type == sf::Event::MouseButtonPressed) {
            if (dragPos) { 
                if (event.mouseButton.button != sf::Mouse::Middle) dragPos = false; // drag ends on mouse click (except for move)
            } else if (currentSelected) { // click off edit
                currentSelected.reset();
            } else if (event.mouseButton.button == sf::Mouse::Left) { // make new point
                Vec2 pos = unvisualize(
                    window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y}));
                sim.addPoint({pos, 1.0F, 0.05F, sim.color});
            } else if (event.mouseButton.button == sf::Mouse::Right && closestPoint) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) { // fast delete
                    removePoint(sim, *closestPoint);
                } else { // edit menu
                    currentSelected = *closestPoint;
                    sim.points[*currentSelected].shape.setFillColor(selectedColour);
                }
            }
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Delete) {
                removePoint(sim, *currentSelected);
            }
        }
    }

    void IMtool() {
        
    }

    void IMedit(Sim& sim, const sf::Vector2i& mousePixPos) {
        Point& point = sim.points[*currentSelected];
        sf::Vector2i pointPixPos;

        if (dragPos == true) { // if dragging
            ImGui::SetMouseCursor(ImGuiMouseCursor_None);
            pointPixPos = mousePixPos;
            point.pos = unvisualize(window.mapPixelToCoords(mousePixPos));
        } else { 
            pointPixPos = window.mapCoordsToPixel(visualize(point.pos));
        }

        // window setup
        ImGui::SetNextWindowPos({static_cast<float>(pointPixPos.x) + 10.0F, static_cast<float>(pointPixPos.y) + 10.0F},
                            ImGuiCond_Always);
        ImGui::Begin("edit", NULL,
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                            ImGuiWindowFlags_NoCollapse);
        ImGui::SetWindowSize({-1.0F, -1.0F}, ImGuiCond_Once);

        // properties
        ImGui_DragDouble("mass", &(point.mass), 0.1F, 0.1, 100.0, "%.1f",
                            ImGuiSliderFlags_AlwaysClamp);

        ImGui::Text("position:");
        ImGui::SameLine();
        if (ImGui::Button("drag")) {
            dragPos = true;
            sf::Mouse::setPosition(pointPixPos, window);
        }
        ImGui::InputDouble("posx", &(point.pos.x));
        ImGui::InputDouble("posy", &(point.pos.y));

        ImGui::Text("velocity:");
        ImGui::InputDouble("velx", &(point.vel.x));
        ImGui::InputDouble("vely", &(point.vel.y));

        // delete
        if (ImGui::Button("delete")) {
            removePoint(sim, *currentSelected);
        }
        ImGui::Text("LShift + RClick (fast delete)");
        ImGui::End();
    }
};