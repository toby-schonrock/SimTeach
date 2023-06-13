#include "EntityManager.hpp"
#include "Fundamentals/Vector2.hpp"
#include "GraphMananager.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"
#include "Sim.hpp"
#include "Spring.hpp"
#include "imgui.h"
#include <cstddef>
#include <iostream>
#include <optional>
#include <string>

static inline const sf::Color selectedPColour = sf::Color::Magenta;
static inline const sf::Color hoverPColour    = sf::Color::Blue;
static inline const sf::Color selectedSColour = sf::Color::Magenta;
static inline const sf::Color hoverSColour    = sf::Color::Blue;

static inline const float width = 120;

const ImGuiWindowFlags editFlags =
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

sf::Vector2f visualize(const Vec2& v);
Vec2         unvisualize(const sf::Vector2f& v);

class Tool {
  public:
    std::string name;
    Tool(sf::RenderWindow& window_, EntityManager& entities_, std::string name_)
        : name(std::move(name_)), window(window_), entities(entities_) {}
    virtual ~Tool()                                               = default;
    virtual void frame(Sim& sim, const sf::Vector2i& mousePixPos) = 0;
    virtual void event(const sf::Event& event)                    = 0;
    virtual void unequip()                                        = 0;
    virtual void ImTool()                                         = 0;
    Tool(const Tool& other)                                       = delete;
    Tool& operator=(const Tool& other) = delete;

  protected:
    virtual void ImEdit(const sf::Vector2i& mousePixPos) = 0;
    void         setPointColor(std::size_t i, sf::Color color) {
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

    sf::RenderWindow& window;
    EntityManager&    entities;
};

class GraphTool : public Tool {
  public:
    GraphTool(sf::RenderWindow& window_, EntityManager& entities_, GraphManager& graphs_,
              const std::string& name_)
        : Tool(window_, entities_, name_), graphs(graphs_) {}
    void frame(Sim& sim, const sf::Vector2i& mousePixPos) override;
    void event(const sf::Event& event) override;
    void unequip() override;
    void ImTool() override;

  private:
    void ImEdit([[maybe_unused]] const sf::Vector2i& mousePixPos) override {}
    void highlightGraph(std::size_t i);
    void resetGraphHighlight(std::size_t i);
    void DrawGraphs();

    enum class State { normal, newG, editG };

    GraphManager& graphs;
    Graph defGraph{0, ObjectType::Point, Property::Position, Component::x, graphs.graphBuffer};
    std::optional<std::size_t> selectedG;
    std::optional<std::size_t> hoveredG;
    std::optional<std::size_t> hoveredS;
    std::optional<std::size_t> selectedS;
    std::optional<std::size_t> hoveredP;
    std::optional<std::size_t> selectedP;
};

class PointTool : public Tool {
  public:
    PointTool(sf::RenderWindow& window_, EntityManager& entities_, const std::string& name_)
        : Tool(window_, entities_, name_) {}
    void frame(Sim& sim, const sf::Vector2i& mousePixPos) override;
    void event(const sf::Event& event) override;
    void unequip() override;
    void ImTool() override;

  private:
    void        removePoint(const std::size_t& pos);
    void        ImEdit(const sf::Vector2i& mousePixPos) override;
    static void pointInputs(Point& point);

    Point                      defPoint  = Point({0.0F, 0.0F}, 1.0F, sf::Color::Red, false);
    std::optional<std::size_t> hoveredP  = std::nullopt;
    std::optional<std::size_t> selectedP = std::nullopt;
    double                     toolRange = 1;
    bool                       dragging  = false;
    bool                       inside    = false;
};

class PolyTool : public Tool {
  public:
    PolyTool(sf::RenderWindow& window_, EntityManager& entities_, const std::string& name_)
        : Tool(window_, entities_, name_) {}
    void frame(Sim& sim, const sf::Vector2i& mousePixPos) override;
    void event(const sf::Event& event) override;
    void unequip() override;
    void ImTool() override {}

  private:
    void ImEdit([[maybe_unused]] const sf::Vector2i& mousePixPos) override {}

    std::vector<Vec2>          verts{{}};
    Polygon                    validPoly{};
    std::optional<std::size_t> deletingP;
    bool                       isDone      = false;
    bool                       isNewConvex = false;
};

class SpringTool : public Tool {
  public:
    SpringTool(sf::RenderWindow& window_, EntityManager& entities_, const std::string& name_)
        : Tool(window_, entities_, name_) {}
    void frame(Sim& sim, const sf::Vector2i& mousePixPos) override;
    void event(const sf::Event& event) override;
    void unequip() override;
    void ImTool() override;

  private:
    void        ImEdit(const sf::Vector2i& mousePixPos) override;
    static void setLineColor(std::array<sf::Vertex, 2>& l, const sf::Color& c);
    void        springInputs(Spring& spring) const;
    void        removeSpring(const std::size_t& pos);

    Spring                     defSpring{10, 1.0, 0.2, 0, 0};
    std::array<sf::Vertex, 2>  line{sf::Vertex{}, sf::Vertex{}};
    std::optional<std::size_t> selectedS = std::nullopt;
    std::optional<std::size_t> hoveredS  = std::nullopt;
    std::optional<std::size_t> selectedP = std::nullopt;
    std::optional<std::size_t> hoveredP  = std::nullopt;
    double                     toolRange = 1;
    bool validHover = false; // wether the current hover is an acceptable second point
    bool autoSizing = false;
};