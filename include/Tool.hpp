#pragma once

#include "Point.hpp"
#include "SFML/Config.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/System/Vector2.hpp"
#include "SFML/Window.hpp"
#include "Sim.hpp"
#include "Vector2.hpp"
#include "imgui.h"
#include <cmath>
#include <iostream>
#include <optional>
#include <string_view>

sf::Vector2f visualize(const Vec2& v);
Vec2         unvisualize(const sf::Vector2f& v);

inline bool ImGui_DragDouble(const char* label, double* v, float v_speed = 1.0f, double v_min = 0,
                             double v_max = 0, const char* format = 0, ImGuiSliderFlags flags = 0) {
    return ImGui::DragScalar(label, ImGuiDataType_Double, v, v_speed, &v_min, &v_max, format,
                             flags);
}

static void HelpMarker(
    const char*
        desc) // taken from dear imgui demo
              // https://github.com/ocornut/imgui/blob/9aae45eb4a05a5a1f96be1ef37eb503a12ceb889/imgui_demo.cpp#L191
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

class Tool {
  protected:
    virtual void IMedit(Sim& sim, const sf::Vector2i& mousePixPos) = 0;

  public:
    sf::RenderWindow& window;
    std::string_view name;
    Tool(sf::RenderWindow& window_, std::string_view name_) : window(window_), name(name_) {}
    virtual void frame(Sim& sim, const sf::Vector2i& mousePixPos) = 0;
    virtual void event(Sim& sim, const sf::Event& event)          = 0;
    virtual void unequip(Sim& sim)                                = 0;
    virtual void IMtool()                                         = 0;
    Tool(const Tool& other)                                       = delete;
    Tool& operator=(const Tool& other) = delete;
};

class T_Slice : public Tool {
  private:
    static inline const sf::Color selectedColour = sf::Color::Blue;
    static constexpr double       sliceRange     = 0.20;
    static constexpr double       deleteRange    = 1;
    std::optional<std::size_t>    closestPoint   = std::nullopt;

  public:
    T_Slice(sf::RenderWindow& window_, std::string_view name_) : Tool(window_, name_) {}

    void frame(Sim& sim, const sf::Vector2i& mousePixPos) override {
        // if sim has no points nothing to do (may change)
        if (sim.points.size() == 0) return;

        if (closestPoint)
            sim.points[*closestPoint]
                .resetColor(); // reset last closest point color as it may not be closest anymore

        Vec2 mousePos = unvisualize(window.mapPixelToCoords(mousePixPos));
        // determine new closest point
        auto close         = sim.findClosestPoint(mousePos);
        closestPoint       = close.first;
        double closestDist = close.second;
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

    void unequip(Sim& sim) override {
        if (closestPoint) {
            sim.points[*closestPoint].resetColor();
            closestPoint.reset();
        }
    }
};

class T_Points : public Tool {
  private:
    static inline const sf::Color selectedColour = sf::Color::Magenta;
    static inline const sf::Color hoverColour    = sf::Color::Blue;
    Point                         defPoint       = Point({0.0F, 0.0F}, 1.0F, 0.05F, sf::Color::Red);
    std::optional<std::size_t>    hoveredP       = std::nullopt;
    std::optional<std::size_t>    selectedP      = std::nullopt;
    double                        toolRange      = 1;
    bool                          dragging       = false;

    void removePoint(Sim& sim, const std::size_t& pos) {
        sim.removePoint(pos);
        if (*selectedP == pos) selectedP.reset();
        if (*hoveredP == pos) hoveredP.reset();
    }

    void IMedit(Sim& sim, const sf::Vector2i& mousePixPos) override {
        Point&       point = sim.points[*selectedP];
        sf::Vector2i pointPixPos;

        if (dragging == true) { // if dragging
            ImGui::SetMouseCursor(ImGuiMouseCursor_None);
            pointPixPos = mousePixPos;
            point.pos   = unvisualize(window.mapPixelToCoords(mousePixPos));
        } else {
            pointPixPos = window.mapCoordsToPixel(visualize(point.pos));
        }

        // window setup
        ImGui::SetNextWindowPos(
            {static_cast<float>(pointPixPos.x) + 10.0F, static_cast<float>(pointPixPos.y) + 10.0F},
            ImGuiCond_Always);
        ImGui::Begin("edit", NULL,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoCollapse);
        ImGui::SetWindowSize({-1.0F, -1.0F}, ImGuiCond_Once);

        // properties

        ImGui::Text("position:");
        ImGui::SameLine();
        if (ImGui::Button("drag")) {
            dragging = true;
            sf::Mouse::setPosition(pointPixPos, window);
        }
        ImGui::InputDouble("posx", &(point.pos.x));
        ImGui::InputDouble("posy", &(point.pos.y));

        PointInputs(point);

        // delete
        if (ImGui::Button("delete")) {
            removePoint(sim, *selectedP);
        }
        ImGui::SameLine();
        HelpMarker("LControl + LClick or Delete");
        ImGui::End();
    }

    void PointInputs(Point& point) {
        ImGui::Text("velocity:");
        ImGui::SameLine();
        if (ImGui::Button("reset")) {
            point.vel = Vec2();
        }
        ImGui::InputDouble("velx", &(point.vel.x));
        ImGui::InputDouble("vely", &(point.vel.y));

        ImGui_DragDouble("mass", &(point.mass), 0.1F, 0.1, 100.0, "%.1f",
                         ImGuiSliderFlags_AlwaysClamp);

        // akward colour translation stuff // remember point.color does not draw that color (needs a
        // Point::updateColor())
        float imcol[4] = {
            static_cast<float>(point.color.r) / 255, static_cast<float>(point.color.g) / 255,
            static_cast<float>(point.color.b) / 255, static_cast<float>(point.color.a) / 255};
        ImGui::ColorPicker4("color", imcol);
        point.color = sf::Color(
            static_cast<uint8_t>(imcol[0] * 255.0F), static_cast<uint8_t>(imcol[1] * 255.0F),
            static_cast<uint8_t>(imcol[2] * 255.0F), static_cast<uint8_t>(imcol[3] * 255.0F));
    }

  public:
    T_Points(sf::RenderWindow& window_, std::string_view name_) : Tool(window_, name_) {}

    void frame(Sim& sim, const sf::Vector2i& mousePixPos) override {
        // if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {

        // }
        // TODO: make custom delete cursor (bin)
        if (sim.points.size() == 0) return;

        if (selectedP) { // edit menu
            IMedit(sim, mousePixPos);
        } else { // if none selected
            if (hoveredP) {
                sim.points[*hoveredP].resetColor(); // reset last closest point color as it may
                hoveredP.reset();                   // not be closest anymore
            }

            // determine new closest point
            auto [closestPoint, closestDist] = sim.findClosestPoint(
                unvisualize(window.mapPixelToCoords(mousePixPos))); // NOLINT yes I did thanks :)
            // color close point for selection
            if (closestDist < toolRange) {
                hoveredP = closestPoint;
                sim.points[*hoveredP].shape.setFillColor(hoverColour);
            }
        }
    }

    void event(Sim& sim, const sf::Event& event) override {
        if (event.type == sf::Event::MouseButtonPressed) {
            if (dragging) {
                if (event.mouseButton.button != sf::Mouse::Middle)
                    dragging = false;                // drag ends on mouse click (except for move)
            } else if (selectedP) {                  // click off edit
                sim.points[*selectedP].resetColor(); // when click off return color to normal
                selectedP.reset();
            } else if (event.mouseButton.button == sf::Mouse::Left) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) { // fast delete
                    if (hoveredP)
                        removePoint(sim, *hoveredP); // only do it if there is a highlighted
                } else {                             // make new point
                    Vec2 pos = unvisualize(
                        window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y}));
                    defPoint.pos = pos;
                    sim.addPoint(defPoint);
                }
            } else if (event.mouseButton.button == sf::Mouse::Right && hoveredP) {
                selectedP = *hoveredP;
                hoveredP.reset();
                sim.points[*selectedP].shape.setFillColor(selectedColour);
            }
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Delete) { // delete key (works for hover and select)
                if (selectedP) {
                    removePoint(sim, *selectedP);
                } else if (hoveredP) {
                    removePoint(sim, *hoveredP);
                }
            }
        }
    }

    void unequip(Sim& sim) override {
        dragging = false;
        if (selectedP) {
            sim.points[*selectedP].resetColor();
            selectedP.reset();
        }
        if (hoveredP) {
            sim.points[*hoveredP].resetColor();
            hoveredP.reset();
        }
    }

    void IMtool() override {
        ImGui_DragDouble("range", &toolRange, 0.1F, 0.1, 100.0, "%.1f",
                         ImGuiSliderFlags_AlwaysClamp);
        if (ImGui::CollapsingHeader(
                "new point",
                ImGuiTreeNodeFlags_DefaultOpen |
                    ImGuiTreeNodeFlags_OpenOnArrow)) { // open on arrow to stop insta close bug
            PointInputs(defPoint);
        }
    }
};

class T_Springs : public Tool {
  private:
    static inline const sf::Color selectedColour = sf::Color::Magenta;
    static inline const sf::Color hoverColour    = sf::Color::Blue;
    Spring                        defSpring{};
    std::array<sf::Vertex, 2>     line;
    std::optional<std::size_t>    selectedS = std::nullopt;
    std::optional<std::size_t>    hoveredS  = std::nullopt;
    std::optional<std::size_t>    selectedP = std::nullopt;
    std::optional<std::size_t>    hoveredP  = std::nullopt;
    double                        toolRange = 1;
    bool validHover = false; // wether the current hover is an acceptable second point

    void IMedit(Sim& sim, const sf::Vector2i& mousePixPos) override {}

  public:
    T_Springs(sf::RenderWindow& window_, std::string_view name_) : Tool(window_, name_) {}

    void frame(Sim& sim, const sf::Vector2i& mousePixPos) override {
        validHover = false;

        if (sim.points.size() == 0) return; // tool is useless if there are no points

        // hover stuff
        if (hoveredP) {
            sim.points[*hoveredP].resetColor(); // reset last closest point color as it may
            hoveredP.reset();                   // not be closest anymore
        }

        // determine new closest point
        sf::Vector2f mousePos = window.mapPixelToCoords(mousePixPos);
        auto [closestPoint, closestDist] =
            sim.findClosestPoint(unvisualize(mousePos)); // NOLINT yes I did thanks :)
        // color close point for selection
        if (closestDist < toolRange &&
            (!selectedP ||
             *selectedP !=
                 closestPoint)) { // if (in range) and (not selected or the selected != closest)
            hoveredP = closestPoint;
            sim.points[*hoveredP].shape.setFillColor(hoverColour);
        }

        if (selectedP) { // if selected
            line[0].position = visualize(sim.points[*selectedP].pos);
            if (hoveredP) {
                auto pos = std::find_if(
                    sim.springs.begin(), sim.springs.end(), // check if spring already exists
                    [hp = *hoveredP, sp = *selectedP](const Spring& s) {
                        return (s.p1 == hp && s.p2 == sp) || (s.p1 == sp && s.p2 == hp);
                    });

                line[1].position = visualize(sim.points[*hoveredP].pos);
                if (pos == sim.springs.end()) {
                    line[0].color = sf::Color::Green;
                    line[1].color = sf::Color::Green;
                    validHover    = true;
                } else {
                    line[0].color = sf::Color::Red;
                    line[1].color = sf::Color::Red;
                }
            } else {
                line[1].position = mousePos;
                line[0].color    = sf::Color::Red;
                line[1].color    = sf::Color::Red;
            }
            window.draw(line.data(), 2, sf::Lines);
            IMedit(sim, mousePixPos);
        }
    }

    void event(Sim& sim, const sf::Event& event) override {
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                if (selectedP) {
                    if (validHover) { // if the hover is valid make new spring
                        sim.points[*hoveredP].resetColor();
                        sim.points[*selectedP].resetColor();
                        hoveredP.reset();
                        selectedP.reset();
                    }
                } else if (hoveredP) {
                    selectedP = *hoveredP;
                    hoveredP.reset();
                    sim.points[*selectedP].shape.setFillColor(selectedColour);
                }
            }
        }
    }

    void unequip(Sim& sim) override {
        if (selectedP) {
            sim.points[*selectedP].resetColor();
            selectedP.reset();
        }
        if (hoveredP) {
            sim.points[*hoveredP].resetColor();
            hoveredP.reset();
        }
        selectedS.reset(); // TODO remember to reset colour
        hoveredS.reset();  // same here
    }

    void IMtool() override {
        ImGui_DragDouble("range", &toolRange, 0.1F, 0.1, 100.0, "%.1f",
                         ImGuiSliderFlags_AlwaysClamp);
    }
};