#pragma once

#include "EntityManager.hpp"
#include "Sim.hpp"
#include "imgui.h"
#include <cmath>
#include <cstddef>
#include <iostream>
#include <optional>

const ImGuiWindowFlags editFlags =
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

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
    virtual void      ImEdit(const sf::Vector2i& mousePixPos) = 0;
    sf::RenderWindow& window;
    EntityManager&    entities;

    void setPointColor(std::size_t i, sf::Color color) {
        i *= 4;
        entities.pointVerts[i].color     = color;
        entities.pointVerts[i + 1].color = color;
        entities.pointVerts[i + 2].color = color;
        entities.pointVerts[i + 3].color = color;
    }

    void resetPointColor(std::size_t i) {
        entities.pointVerts[i * 4].color     = entities.points[i].color;
        entities.pointVerts[i * 4 + 1].color = entities.points[i].color;
        entities.pointVerts[i * 4 + 2].color = entities.points[i].color;
        entities.pointVerts[i * 4 + 3].color = entities.points[i].color;
    }

    void setSpringColor(std::size_t i, sf::Color color) {
        i *= 2;
        entities.springVerts[i].color     = color;
        entities.springVerts[i + 1].color = color;
    }

  public:
    std::string name;

    Tool(sf::RenderWindow& window_, EntityManager& entities_, std::string name_)
        : window(window_), entities(entities_), name(std::move(name_)) {}

    virtual ~Tool()                                               = default;
    virtual void frame(Sim& sim, const sf::Vector2i& mousePixPos) = 0;
    virtual void event(const sf::Event& event)                    = 0;
    virtual void unequip()                                        = 0;
    virtual void ImTool()                                         = 0;
    Tool(const Tool& other)                                       = delete;
    Tool& operator=(const Tool& other) = delete;
};

class PointTool : public Tool {
  private:
    static inline const sf::Color selectedColour = sf::Color::Magenta;
    static inline const sf::Color hoverColour    = sf::Color::Blue;
    Point                         defPoint       = Point({0.0F, 0.0F}, 1.0F, sf::Color::Red);
    std::optional<std::size_t>    hoveredP       = std::nullopt;
    std::optional<std::size_t>    selectedP      = std::nullopt;
    double                        toolRange      = 1;
    bool                          dragging       = false;
    bool                          inside         = false;

    void removePoint(const std::size_t& pos) {
        entities.rmvPoint(pos);
        if (*selectedP == pos) selectedP.reset();
        if (*hoveredP == pos) hoveredP.reset();
    }

    void ImEdit(const sf::Vector2i& mousePixPos) override {
        Point&       point = entities.points[*selectedP];
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
        ImGui::Begin("edit point", NULL,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoCollapse);
        ImGui::SetWindowSize({-1.0F, -1.0F}, ImGuiCond_Always);

        // properties

        ImGui::Text("position:");
        ImGui::SameLine();
        if (ImGui::Button("drag")) {
            dragging = true;
            sf::Mouse::setPosition(pointPixPos, window);
        }
        ImGui::InputDouble("posx", &(point.pos.x));
        ImGui::InputDouble("posy", &(point.pos.y));

        pointInputs(point);

        // set as tools settings
        if (ImGui::Button("copy")) {
            defPoint = entities.points[*selectedP];
        }
        ImGui::SameLine();
        HelpMarker("Copy settings to the tool");

        // delete
        if (ImGui::Button("delete")) {
            removePoint(*selectedP);
        }
        ImGui::SameLine();
        HelpMarker("LControl + LClick or Delete");
        ImGui::End();
    }

    static void pointInputs(Point& point) {
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
    PointTool(sf::RenderWindow& window_, EntityManager& entities_, const std::string& name_)
        : Tool(window_, entities_, name_) {}

    void frame(Sim& sim, const sf::Vector2i& mousePixPos) override {
        Vec2 mousePos = unvisualize(window.mapPixelToCoords(mousePixPos));
        auto poly     = std::find_if(entities.polys.begin(), entities.polys.end(),
                                     [mousePos](const Polygon& p) { return p.isContained(mousePos); });

        if (poly == entities.polys.end()) {
            inside = false;
        } else {
            inside = true;
            ImGui::SetTooltip("Cant place point in polygon");
        }

        if (entities.points.size() == 0) return;

        if (selectedP) { // edit menu
            ImEdit(mousePixPos);
        } else { // if none selected
            if (hoveredP) {
                resetPointColor(*hoveredP); // reset last closest point color as it may
                hoveredP.reset();           // not be closest anymore
            }

            // determine new closest point
            auto [closestPoint, closestDist] =
                sim.findClosestPoint(mousePos); // NOLINT yes I did thanks :)
            // color close point for selection
            if (closestDist < toolRange) {
                hoveredP = closestPoint;
                setPointColor(*hoveredP, hoverColour);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
                    ImGui::SetTooltip("Click to delete");
                }
            }
        }
    }

    void event(const sf::Event& event) override {
        if (event.type == sf::Event::MouseButtonPressed) {
            if (dragging) {
                if (event.mouseButton.button != sf::Mouse::Middle)
                    dragging = false;        // drag ends on mouse click (except for move)
            } else if (selectedP) {          // click off edit
                resetPointColor(*selectedP); // when click off return color to normal
                selectedP.reset();
            } else if (event.mouseButton.button == sf::Mouse::Left) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) { // fast delete
                    if (hoveredP) removePoint(*hoveredP); // only do it if there is a highlighted
                } else if (!inside) {                     // make new point
                    Vec2 pos = unvisualize(
                        window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y}));
                    defPoint.pos = pos;
                    entities.addPoint(defPoint);
                }
            } else if (event.mouseButton.button == sf::Mouse::Right && hoveredP) { // select point
                selectedP = *hoveredP;
                hoveredP.reset();
                setPointColor(*selectedP, selectedColour);
            }
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Delete) { // delete key (works for hover and select)
                if (selectedP) {
                    removePoint(*selectedP);
                } else if (hoveredP) {
                    removePoint(*hoveredP);
                }
            }
        }
    }

    void unequip() override {
        dragging = false;
        if (selectedP) {
            resetPointColor(*selectedP);
            selectedP.reset();
        }
        if (hoveredP) {
            resetPointColor(*hoveredP);
            hoveredP.reset();
        }
    }

    void ImTool() override {
        ImGui_DragDouble("range", &toolRange, 0.1F, 0.1, 100.0, "%.1f",
                         ImGuiSliderFlags_AlwaysClamp);
        if (ImGui::CollapsingHeader(
                "new point",
                ImGuiTreeNodeFlags_DefaultOpen |
                    ImGuiTreeNodeFlags_OpenOnArrow)) { // open on arrow to stop insta close bug
            pointInputs(defPoint);
        }
    }
};

class SpringTool : public Tool {
  private:
    static inline const sf::Color selectedPColour = sf::Color::Magenta;
    static inline const sf::Color hoverPColour    = sf::Color::Blue;
    static inline const sf::Color selectedSColour = sf::Color::Magenta;
    static inline const sf::Color hoverSColour    = sf::Color::Blue;
    Spring                        defSpring{10, 1.0, 0.2, 0, 0};
    std::array<sf::Vertex, 2>     line{sf::Vertex{}, sf::Vertex{}};
    std::optional<std::size_t>    selectedS = std::nullopt;
    std::optional<std::size_t>    hoveredS  = std::nullopt;
    std::optional<std::size_t>    selectedP = std::nullopt;
    std::optional<std::size_t>    hoveredP  = std::nullopt;
    double                        toolRange = 1;
    bool validHover = false; // wether the current hover is an acceptable second point

    void ImEdit([[maybe_unused]] const sf::Vector2i& mousePixPos) override {
        Spring& spring    = entities.springs[*selectedS];
        Vec2    springPos = (entities.points[spring.p1].pos + entities.points[spring.p2].pos) / 2;
        sf::Vector2i springPixPos = window.mapCoordsToPixel(visualize(springPos));
        ImGui::SetNextWindowPos({static_cast<float>(springPixPos.x) + 10.0F,
                                 static_cast<float>(springPixPos.y) + 10.0F},
                                ImGuiCond_Always);
        ImGui::Begin("edit spring", NULL, editFlags);
        ImGui::SetWindowSize({-1.0F, -1.0F}, ImGuiCond_Always);
        springInputs(spring);

        // set as tools settings
        if (ImGui::Button("set default")) {
            defSpring = entities.springs[*selectedS];
        }
        ImGui::SameLine();
        HelpMarker("Copys settings to the tool");

        // delete
        if (ImGui::Button("delete")) {
            removeSpring(*selectedS);
        }
        ImGui::SameLine();
        HelpMarker("LControl + LClick or Delete");
        ImGui::End();
    }

    static void setLineColor(std::array<sf::Vertex, 2>& l, const sf::Color& c) {
        l[0].color = c;
        l[1].color = c;
    }

    void springInputs(Spring& spring) const {
        ImGui::InputDouble("spring constant", &spring.springConst);
        ImGui::InputDouble("damping factor", &spring.dampFact);
        ImGui::InputDouble("natural length", &spring.stablePoint);
    }

    void removeSpring(const std::size_t& pos) {
        entities.rmvSpring(pos);
        if (*selectedS == pos) selectedS.reset();
        if (*hoveredS == pos) hoveredS.reset();
    }

  public:
    SpringTool(sf::RenderWindow& window_, EntityManager& entities_, const std::string& name_)
        : Tool(window_, entities_, name_) {}

    void frame(Sim& sim, const sf::Vector2i& mousePixPos) override {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
            ImGui::SetTooltip("Click to delete");
        }
        if (entities.points.size() == 0) return; // tool is useless if there are no points

        // hover stuff
        if (hoveredP) {
            resetPointColor(*hoveredP); // reset last closest point color as it may
            hoveredP.reset();           // not be closest anymore
        }
        if (hoveredS) {
            setSpringColor(                   // reset last closest line color as it may  not be
                *hoveredS, sf::Color::White); // closest anymore
            hoveredS.reset();
        }

        if (selectedS) { // if in spring editing mode
            ImEdit(mousePixPos);
        } else { // if in normal mode
            sf::Vector2f mousePos = window.mapPixelToCoords(mousePixPos);
            // determine new closest point (needs to happend wether adding or not)
            auto [closestP, closestPDist] = sim.findClosestPoint(unvisualize(mousePos));
            // color close point for selection
            if (closestPDist < toolRange &&
                (!selectedP ||
                 *selectedP !=
                     closestP)) { // if (in range) and (not selected or the selected != closest)
                hoveredP = closestP;
                setPointColor(*hoveredP, hoverPColour);
            }

            if (selectedP) { // if selected point (in making spring mode)
                line[0].position = visualize(entities.points[*selectedP].pos);
                if (hoveredP) {
                    auto pos = std::find_if(
                        entities.springs.begin(),
                        entities.springs.end(), // check if spring already exists
                        [hp = *hoveredP, sp = *selectedP](const Spring& s) {
                            return (s.p1 == hp && s.p2 == sp) || (s.p1 == sp && s.p2 == hp);
                        });

                    line[1].position = visualize(entities.points[*hoveredP].pos);
                    if (pos == entities.springs.end()) {
                        setLineColor(line, sf::Color::Green);
                        validHover = true;
                    } else {
                        setLineColor(line, sf::Color::Red);
                        validHover = false;
                    }
                } else {
                    line[1].position = mousePos;
                    setLineColor(line, sf::Color::Red);
                    validHover = false;
                }
                window.draw(line.data(), 2, sf::Lines);
            } else { // if not making a spring (!selectedP)
                // determine new closest spring
                if (!entities.springs.empty()) {
                    auto [closestS, closestSDist] = sim.findClosestSpring(unvisualize(mousePos));
                    if (closestSDist < toolRange) {
                        hoveredS = closestS;
                        setSpringColor(*hoveredS, hoverSColour);
                    }
                }
            }
        }
    }

    void event(const sf::Event& event) override {
        if (event.type == sf::Event::MouseButtonPressed) {
            if (selectedS) {
                setSpringColor(*selectedS, sf::Color::White);
                selectedS.reset(); // unselect (close edit)
            } else if (selectedP && event.mouseButton.button !=
                                        sf::Mouse::Left) { // if not a left click unselect point
                resetPointColor(*selectedP);
                selectedP.reset();
            } else if (event.mouseButton.button == sf::Mouse::Left) {
                if (selectedP) {
                    if (validHover) { // if the hover is valid make new spring
                        defSpring.p1 = *selectedP;
                        defSpring.p2 = *hoveredP;
                        entities.addSpring(defSpring);
                        resetPointColor(*selectedP); // hovered will be reset anyway
                        selectedP.reset();
                    }
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) { // fast delete
                    if (hoveredS) removeSpring(*hoveredS); // only do it if there is a highlighted
                } else if (hoveredP) {
                    selectedP = *hoveredP;
                    setPointColor(*selectedP, selectedPColour);
                    hoveredP.reset();
                }
            } else if (event.mouseButton.button == sf::Mouse::Right) { // selecting a spring
                if (hoveredS) {
                    selectedS = hoveredS;
                    hoveredS.reset();
                    setSpringColor(*selectedS, selectedSColour);
                }
            }
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Delete) { // delete key (works for hover and select)
                if (selectedP) {
                    removeSpring(*selectedS);
                } else if (hoveredP) {
                    removeSpring(*hoveredS);
                }
            }
        }
    }

    void unequip() override {
        if (selectedP) {
            resetPointColor(*selectedP);
            selectedP.reset();
        }
        if (hoveredP) {
            resetPointColor(*hoveredP);
            hoveredP.reset();
        }
        if (selectedS) {
            setSpringColor(*selectedS, sf::Color::White);
            selectedS.reset();
        }
        if (hoveredS) {
            setSpringColor(*hoveredS, sf::Color::White);
            hoveredS.reset();
        }
    }

    void ImTool() override {
        ImGui_DragDouble("range", &toolRange, 0.1F, 0.1, 100.0, "%.1f",
                         ImGuiSliderFlags_AlwaysClamp);
        if (ImGui::CollapsingHeader(
                "new spring",
                ImGuiTreeNodeFlags_DefaultOpen |
                    ImGuiTreeNodeFlags_OpenOnArrow)) { // open on arrow to stop insta close bug
            springInputs(defSpring);
        }
    }
};

class CustomPolyTool : public Tool {
  private:
    Polygon                   poly{};
    std::array<sf::Vertex, 2> line{};
    Vec2                      newPoint{};
    bool                      convex = false;
    bool                      inside = false;

    void ImEdit([[maybe_unused]] const sf::Vector2i& mousePixPos) override {}

  public:
    CustomPolyTool(sf::RenderWindow& window_, EntityManager& entities_, const std::string& name_)
        : Tool(window_, entities_, name_) {}

    void frame([[maybe_unused]] Sim& sim, const sf::Vector2i& mousePixPos) override {
        sf::Vector2f mousePos = window.mapPixelToCoords(mousePixPos);
        newPoint              = unvisualize(mousePos);

        if (!poly.edges.empty()) {
            if (poly.edges.size() > 2) {
                convex = poly.isConvex();
                if (convex) {
                    poly.shape.setFillColor(sf::Color::White);
                } else {
                    poly.shape.setFillColor(sf::Color::Red);
                }
                if (!inside) poly.rmvEdge();         // no need to repeat if already hovering inside
                inside = poly.isContained(newPoint); // update
                if (inside) {
                    poly.shape.setFillColor(sf::Color::Green);
                } else {
                    poly.addEdge(newPoint);
                }
            } else {
                poly.edges.back().p1(newPoint);
                poly.edges[poly.edges.size() - 2].p2(newPoint);
            }
            poly.draw(window, true);
        }
    }

    void event(const sf::Event& event) override {
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) { // new vert
                if (inside && poly.edges.size() > 2) {         // if green
                    poly.shape.setFillColor(sf::Color::White);
                    poly.boundsUp();
                    entities.polys.push_back(poly);
                    poly = Polygon{};
                } else if (convex || poly.edges.size() < 3) { // if not red
                    if (poly.edges.empty()) {
                        poly = Polygon({newPoint, {}});
                    } else {
                        poly.addEdge(newPoint);
                    }
                }
            } else if (event.mouseButton.button == sf::Mouse::Right) {
                if (!poly.edges.empty()) poly.rmvEdge();
            }
        }
    }
    void unequip() override { poly = Polygon{}; }
    void ImTool() override {}
};

class GraphTool : public Tool {
  private:
    std::optional<std::size_t> selected;
    // static inline const ImPlot::color

    void ImEdit(const sf::Vector2i& mousePixPos) override {}

    void DrawGraphs() {
        ImGui::Begin("Graphs");
        for (Graph& g: entities.graphs) {
            g.draw();
            ImGui::Text("Is hovered: %s", ImGui::IsItemHovered() ? "true" : "false");
        }
        ImGui::End();
    }

  public:
    GraphTool(sf::RenderWindow& window_, EntityManager& entities_, const std::string& name_)
        : Tool(window_, entities_, name_) {}

    void frame(Sim& sim, const sf::Vector2i& mousePixPos) override { DrawGraphs(); }

    virtual void event(const sf::Event& event) {}
    virtual void unequip() {}
    virtual void ImTool() {}

    void interface() {}
};