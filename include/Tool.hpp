#pragma once

#include "Graph.hpp"
#include "GraphMananager.hpp"
#include "ImguiHelpers.hpp"
#include "Polygon.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Window.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Window/Mouse.hpp"
#include "Sim.hpp"
#include "Spring.hpp"
#include "Vector2.hpp"
#include "imgui.h"
#include <cstddef>
#include <iostream>
#include <optional>

static inline const sf::Color selectedPColour = sf::Color::Magenta;
static inline const sf::Color hoverPColour    = sf::Color::Blue;
static inline const sf::Color selectedSColour = sf::Color::Magenta;
static inline const sf::Color hoverSColour    = sf::Color::Blue;

static inline const float width = 120;

static inline const ImVec4 coloredText = {1, 1, 0, 1};

const ImGuiWindowFlags editFlags =
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

sf::Vector2f visualize(const Vec2& v);
Vec2         unvisualize(const sf::Vector2f& v);

bool ImGui_DragDouble(const char* label, double* v, float v_speed, double v_min, double v_max,
                      const char* format, ImGuiSliderFlags flags);

void HelpMarker(const char* desc);

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
    Point                      defPoint  = Point({0.0F, 0.0F}, 1.0F, sf::Color::Red, false);
    std::optional<std::size_t> hoveredP  = std::nullopt;
    std::optional<std::size_t> selectedP = std::nullopt;
    double                     toolRange = 1;
    bool                       dragging  = false;
    bool                       inside    = false;

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
        ImGui::SetNextItemWidth(width);
        Vec2F posTemp = Vec2F(point.pos);
        ImGui::DragFloat2("Position", &posTemp.x, 0.01F);
        point.pos = Vec2(posTemp);
        ImGui::SameLine();
        if (ImGui::Button("drag")) {
            dragging = true;
            sf::Mouse::setPosition(pointPixPos, window);
        }

        pointInputs(point);

        // set as tools settings
        if (ImGui::Button("set as default")) {
            defPoint = entities.points[*selectedP];
        }
        ImGui::SameLine();
        HelpMarker("Copy settings to the spring tool");

        // delete
        if (ImGui::Button("delete")) {
            removePoint(*selectedP);
        }
        ImGui::SameLine();
        HelpMarker("LControl + LClick or Delete");
        ImGui::End();
    }

    static void pointInputs(Point& point) {
        ImGui::Checkbox("Fixed", &point.fixed);
        if (point.fixed) {
            ImGui::BeginDisabled();
            point.vel = Vec2();
        }
        ImGui::SetNextItemWidth(width);
        Vec2F velTemp = Vec2F(point.vel);
        ImGui::DragFloat2("Velocity", &velTemp.x, 0.01F);
        point.vel = Vec2(velTemp);
        ImGui::SameLine();
        if (ImGui::Button("reset")) {
            point.vel = Vec2();
        }

        ImGui::SetNextItemWidth(width);
        ImGui_DragDouble("Mass", &(point.mass), 0.1F, 0.1, 100.0, "%.1f",
                         ImGuiSliderFlags_AlwaysClamp);

        if (point.fixed) {
            ImGui::EndDisabled();
        }

        // akward colour translation stuff // remember point.color does not draw that color (needs a
        // Point::updateColor())
        float imcol[4] = {
            static_cast<float>(point.color.r) / 255, static_cast<float>(point.color.g) / 255,
            static_cast<float>(point.color.b) / 255, static_cast<float>(point.color.a) / 255};
        ImGui::ColorPicker4("Color", imcol);
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
                setPointColor(*hoveredP, hoverPColour);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
                    ImGui::SetTooltip("Click to delete");
                else if (inside)
                    ImGui::SetTooltip("Cant place point in polygon");
            }
        }
    }

    void event(const sf::Event& event) override {
        if (event.type == sf::Event::MouseButtonPressed && !ImGui::GetIO().WantCaptureMouse) {
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
                setPointColor(*selectedP, selectedPColour);
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
        ImGui::SetNextItemWidth(width);
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
    Spring                     defSpring{10, 1.0, 0.2, 0, 0};
    std::array<sf::Vertex, 2>  line{sf::Vertex{}, sf::Vertex{}};
    std::optional<std::size_t> selectedS = std::nullopt;
    std::optional<std::size_t> hoveredS  = std::nullopt;
    std::optional<std::size_t> selectedP = std::nullopt;
    std::optional<std::size_t> hoveredP  = std::nullopt;
    double                     toolRange = 1;
    bool validHover = false; // wether the current hover is an acceptable second point
    bool autoSizing = false;

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
        ImGui::SetNextItemWidth(100.0F);
        ImGui::InputDouble("natural length", &spring.naturalLength, 0, 0, "%.3f");

        // set as tools settings
        if (ImGui::Button("set as default")) {
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
        ImGui::SetNextItemWidth(100.0F);
        ImGui::InputDouble("spring constant", &spring.springConst, 0, 0, "%.3f");
        ImGui::SetNextItemWidth(100.0F);
        ImGui::InputDouble("damping factor", &spring.dampFact, 0, 0, "%.3f");
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
                        ImGui::SetTooltip("Spring already exists");
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
        if (event.type == sf::Event::MouseButtonPressed && !ImGui::GetIO().WantCaptureMouse) {
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
                        entities.addSpring(defSpring);
                        Spring& s = entities.springs.back();
                        s.p1      = *selectedP;
                        s.p2      = *hoveredP;
                        if (autoSizing)
                            s.naturalLength =
                                (entities.points[s.p1].pos - entities.points[s.p2].pos).mag();
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
        ImGui_DragDouble("Range", &toolRange, 0.1F, 0.1, 100.0, "%.1f",
                         ImGuiSliderFlags_AlwaysClamp);
        ImGui::Checkbox("Auto sizing", &autoSizing);
        ImGui::SameLine();
        HelpMarker(
            "Springs natural length will be auto set to the distance between the two points");
        if (ImGui::CollapsingHeader(
                "new spring",
                ImGuiTreeNodeFlags_DefaultOpen |
                    ImGuiTreeNodeFlags_OpenOnArrow)) { // open on arrow to stop insta close bug
            springInputs(defSpring);
            if (autoSizing) ImGui::BeginDisabled();
            ImGui::SetNextItemWidth(100.0F);
            ImGui::InputDouble("natural length", &defSpring.naturalLength, 0, 0, "%.3f");
            if (autoSizing) ImGui::EndDisabled();
        }
    }
};

class CustomPolyTool : public Tool {
  private:
    Polygon                    poly{};
    std::array<sf::Vertex, 2>  line{};
    std::optional<std::size_t> hoveredP;
    Vec2                       newPoint{};
    bool                       convex = false;
    bool                       inside = false;

    void ImEdit([[maybe_unused]] const sf::Vector2i& mousePixPos) override {}

  public:
    CustomPolyTool(sf::RenderWindow& window_, EntityManager& entities_, const std::string& name_)
        : Tool(window_, entities_, name_) {}

    void frame([[maybe_unused]] Sim& sim, const sf::Vector2i& mousePixPos) override {
        sf::Vector2f mousePos = window.mapPixelToCoords(mousePixPos);
        newPoint              = unvisualize(mousePos);

        if (hoveredP) {
            entities.polys[*hoveredP].shape.setFillColor(sf::Color::White);
            hoveredP.reset();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
            for (std::size_t i = 0; i != entities.polys.size(); ++i) {
                if (entities.polys[i].isBounded(unvisualize(mousePos)) &&
                    entities.polys[i].isContained(unvisualize(mousePos))) {
                    hoveredP = i;
                }
            }
            if (hoveredP) {
                entities.polys[*hoveredP].shape.setFillColor(sf::Color::Red);
                ImGui::SetTooltip("Click to delete");
            }
        }

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
        if (event.type == sf::Event::MouseButtonPressed && !ImGui::GetIO().WantCaptureMouse) {
            if (event.mouseButton.button == sf::Mouse::Left) { // new vert
                if (hoveredP) {                                // delete
                    entities.graphs.erase(entities.graphs.begin() +
                                          static_cast<std::ptrdiff_t>(*hoveredP));
                } else if (inside && poly.edges.size() > 2) { // if green
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
    enum class State { normal, newG, editG };

    GraphManager&              graphs;
    std::optional<std::size_t> selectedG;
    std::optional<std::size_t> hoveredG;
    std::optional<std::size_t> hoveredS;
    std::optional<std::size_t> graphS2;
    std::optional<std::size_t> hoveredP;
    std::optional<std::size_t> graphP2;
    bool                       makingIndexDiff;

    // new graph properties
    bool                       makingNew;
    std::optional<ObjectType>  type;
    std::optional<std::size_t> index;
    std::optional<Property>    prop;
    std::optional<Component>   comp;

    // static inline const ImPlot::color

    void ImEdit(const sf::Vector2i& mousePixPos) override {}

    void resetNewGraph() {
        if (index) {
            if (type == ObjectType::Point) {
                resetPointColor(*index);
            } else {
                setSpringColor(*index, sf::Color::White);
            }
        }
        makingIndexDiff = false;
        makingNew       = false;
        type            = std::nullopt;
        index           = std::nullopt;
        prop            = std::nullopt;
        comp            = std::nullopt;
    }

    void highlightGraph(std::size_t i) {
        Graph& g = entities.graphs[i];
        if (g.type == ObjectType::Point) {
            setPointColor(g.y, selectedPColour);
            if (g.y2) setPointColor(*g.y2, selectedPColour);
        } else {
            setSpringColor(g.y, selectedSColour);
            if (g.y2) setSpringColor(*g.y2, selectedSColour);
        }
    }

    void resetGraphHighlight(std::size_t i) {
        Graph& g = entities.graphs[i];
        if (g.type == ObjectType::Point) {
            resetPointColor(g.y);
            if (g.y2) resetPointColor(*g.y2);
        } else {
            setSpringColor(g.y, sf::Color::White);
            if (g.y2) setSpringColor(*g.y2, sf::Color::White);
        }
    }

    void DrawGraphs() {
        std::optional<std::size_t> newHover = std::nullopt;
        if (makingNew) ImGui::SetNextWindowCollapsed(false);
        ImGui::Begin("Graphs");
        if (!ImGui::IsWindowCollapsed()) {
            for (std::size_t i = 0; i != entities.graphs.size(); ++i) {
                if (selectedG && i == *selectedG)
                    ImPlot::PushStyleColor(ImPlotCol_PlotBg, {0.0F, 1.0F, 0.537F, 0.27F});
                else if (hoveredG && i == *hoveredG)
                    ImPlot::PushStyleColor(ImPlotCol_PlotBg, {0.133F, 0.114F, 0.282F, 0.2F});
                entities.graphs[i].draw(i);
                if (ImGui::IsItemHovered()) {
                    newHover = i;
                }
                if ((hoveredG && i == *hoveredG) || (selectedG && i == *selectedG))
                    ImPlot::PopStyleColor();
            }
        }
        if (makingNew) {
            makingNew = false;
            ImGui::SetScrollHereY(1.0F);
        }
        hoveredG = newHover;
        ImGui::End();
    }

  public:
    GraphTool(sf::RenderWindow& window_, EntityManager& entities_, GraphManager& graphs_,
              const std::string& name_)
        : Tool(window_, entities_, name_), graphs(graphs_) {}

    void frame(Sim& sim, const sf::Vector2i& mousePixPos) override {
        if (hoveredP) { // color resets
            resetPointColor(*hoveredP);
            hoveredP.reset();
        }
        if (hoveredS) {
            setSpringColor(*hoveredS, sf::Color::White);
            hoveredS.reset();
        }
        if (index) { // Point selected
            if (type == ObjectType::Point) {
                setPointColor(*index, selectedPColour);
            } else { // other option should only be spring
                setSpringColor(*index, selectedSColour);
            }
        }

        if (!makingIndexDiff &&
            (!makingNew || (makingNew && comp))) // if normal or just finished making a new graph
            DrawGraphs();
        else {          // if making new
            if (prop) { // Component selection
                if (ImGui::BeginPopupContextItem("Component")) {
                    bool add = true;
                    if (ImGui::Selectable("Magnitude")) {
                        comp = Component::vec;
                    } else if (ImGui::Selectable("x-component")) {
                        comp = Component::x;
                    } else if (ImGui::Selectable("y-component")) {
                        comp = Component::y;
                    } else
                        add = false; // if none clicked
                    if (add) {
                        graphs.addGraph(*index, *type, *prop, *comp);
                        selectedG = entities.graphs.size() - 1;
                        resetNewGraph();
                        highlightGraph(*selectedG);
                    }
                    ImGui::EndPopup();
                }
                ImGui::OpenPopup("Component");
            } else if (index) { // Property selection
                if (ImGui::BeginPopupContextItem("Property")) {
                    if (*type == ObjectType::Spring) {
                        if (ImGui::Selectable("Length")) prop = Property::Length;
                        if (ImGui::Selectable("Extension")) prop = Property::Extension;
                        if (ImGui::Selectable("Force")) prop = Property::Force;
                    } else if (*type == ObjectType::Point) {
                        if (ImGui::Selectable("Position")) prop = Property::Position;
                        if (ImGui::Selectable("Velocity")) prop = Property::Velocity;
                    }
                    ImGui::EndPopup();
                }
                ImGui::OpenPopup("Property");
            } else if (type) { // selecting an object
                Vec2 mousePos = unvisualize(window.mapPixelToCoords(mousePixPos));
                ImGui::SetTooltip("Select a %s", getTypeLbl(*type).c_str());
                if (type == ObjectType::Point) {
                    auto [closestP, closestSDist] = sim.findClosestPoint(mousePos);
                    hoveredP                      = closestP;
                    setPointColor(*hoveredP, hoverPColour);
                } else { // other option should only be spring
                    auto [closestS, closestSDist] = sim.findClosestSpring(mousePos);
                    hoveredS                      = closestS;
                    setSpringColor(*hoveredS, hoverSColour);
                }
            } else if (makingNew) { // first option
                if (ImGui::BeginPopupContextItem("Type")) {
                    if (ImGui::Selectable("Point")) type = ObjectType::Point;
                    if (ImGui::Selectable("Spring")) type = ObjectType::Spring;
                    ImGui::EndPopup();
                }
                ImGui::OpenPopup("Type");
            } else if (makingIndexDiff) { // selecting the object to take the difference from
                ImGui::SetTooltip("Select a %s",
                                  getTypeLbl(entities.graphs[*selectedG].type).c_str());
                Vec2 mousePos = unvisualize(window.mapPixelToCoords(mousePixPos));
                if (entities.graphs[*selectedG].type == ObjectType::Point) {
                    auto [closestP, closestSDist] = sim.findClosestPoint(mousePos);
                    hoveredP                      = closestP;
                    setPointColor(*hoveredP, hoverPColour);
                } else { // other option should only be spring
                    auto [closestS, closestSDist] = sim.findClosestSpring(mousePos);
                    hoveredS                      = closestS;
                    setSpringColor(*hoveredS, hoverSColour);
                }
            }
        }
    }

    void event(const sf::Event& event) override {
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                if (hoveredG) {                                     // new graph selected
                    if (selectedG) resetGraphHighlight(*selectedG); // reset old graph
                    selectedG = *hoveredG;
                    highlightGraph(*selectedG);
                } else if (makingIndexDiff &&
                           !ImGui::GetIO().WantCaptureMouse) { // selecting object for diff graph
                    makingIndexDiff = false;
                    entities.graphs[*selectedG].y2 =
                        entities.graphs[*selectedG].type == ObjectType::Point ? *hoveredP
                                                                              : *hoveredS;
                    highlightGraph(*selectedG);
                    hoveredP.reset(); // to prevent unhighliting on new point creation
                } else if (selectedG && !ImGui::GetIO().WantCaptureMouse) { // deselect graph
                    resetGraphHighlight(*selectedG);
                    selectedG.reset();
                } else if (type && !index &&
                           !ImGui::GetIO().WantCaptureMouse) { // selecting object for new graph
                    if (type == ObjectType::Point)
                        index = hoveredP;
                    else
                        index = hoveredS;
                }
            }
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                resetNewGraph();
            }
        }
    }

    void unequip() override {
        resetNewGraph();
        if (hoveredG) {
            resetGraphHighlight(*hoveredG);
            hoveredG.reset();
        }
        if (selectedG) {
            resetGraphHighlight(*selectedG);
            selectedG.reset();
        }
    }

    void ImTool() override {
        if (graphs.hasDumped) ImGui::BeginDisabled();
        if (ImGui::Button("save data")) {
            graphs.dumpData();
        }
        if (graphs.hasDumped) {
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Data already dumped check the saved folder");
            ImGui::EndDisabled();
        }

        if (makingNew) ImGui::BeginDisabled();
        if (ImGui::Button("make new")) {
            resetNewGraph();
            if (selectedG) {
                resetGraphHighlight(*selectedG);
                selectedG.reset();
            }
            makingNew = true;
        } else if (makingNew)
            ImGui::EndDisabled(); // else if to prevent additional EndDisabled() on creation

        if (!selectedG) {
            ImGui::BeginDisabled();
        }
        ImGui::SetNextItemOpen(selectedG.has_value());
        if (ImGui::CollapsingHeader("Graph properties", ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
            if (selectedG) {
                Graph& g = entities.graphs[*selectedG];
                ImGui::Text("Object type -");
                ImGui::SameLine();
                ImGui::TextColored(coloredText, "%s", getTypeLbl(g.type).c_str());

                int c = static_cast<int>(g.comp);
                ImGui::Combo("Component", &c, CompLbl.data(), CompLbl.size());
                g.comp = static_cast<Component>(c);

                int p;
                if (g.type == ObjectType::Point) {
                    p = static_cast<int>(g.prop);
                    ImGui::Combo("Property", &p, PropLbl.data(), 2);
                } else {
                    p = static_cast<int>(g.prop) - 2;
                    ImGui::Combo("Property", &p, PropLbl.data() + 2,
                                 3); // offset for spring properties
                    p += 2;
                }
                g.prop = static_cast<Property>(p);

                ImGui::Text("Index");
                ImGui::SameLine();
                ImGui::TextColored(coloredText, "%zu", g.y);

                enum class DiffState { None, Index, Const };
                constexpr static std::array DiffStatusLbl{"None", "Index", "Const"};

                DiffState oldState =
                    g.y2 ? DiffState::Index : (g.constDiff ? DiffState::Const : DiffState::None);
                if (makingIndexDiff) oldState = DiffState::Index;
                int temp = static_cast<int>(oldState);
                ImGui::Combo("Difference", &temp, DiffStatusLbl.data(), DiffStatusLbl.size());
                DiffState newState = static_cast<DiffState>(temp);
                if (newState != oldState) { // if state changed
                    if (newState == DiffState::Index) {
                        makingIndexDiff = true;
                    }
                    if (newState == DiffState::Const) {
                        g.constDiff = Vec2F{};
                    }
                    if (oldState == DiffState::Index) {
                        if (g.type == ObjectType::Point)
                            resetPointColor(*g.y2);
                        else
                            setSpringColor(*g.y2, sf::Color::White);
                        g.y2.reset();
                    }
                }
                if (oldState == DiffState::Index) { // old state to allow changing index
                    ImGui::Text("Diff Index");
                    ImGui::SameLine();
                    if (g.y2) {
                        ImGui::TextColored(coloredText, "%zu", *g.y2);
                    } else {
                        ImGui::TextColored((selectedPColour), "%zu",
                                           g.type == ObjectType::Point
                                               ? *hoveredP
                                               : *hoveredS); // TODO make a new color
                    }
                } else if (oldState == DiffState::Const) {
                    ImGui::DragFloat2("Value", &g.constDiff->x);
                }
            }
        }
        if (!selectedG) ImGui::EndDisabled();
    }

    void interface() {}
};