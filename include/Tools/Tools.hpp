#include "EntityManager.hpp"
#include "Fundamentals/Vector2.hpp"
#include "Graph.hpp"
#include "GraphMananager.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Graphics/Color.hpp"
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
    Tool& operator=(const Tool& other)                            = delete;

  protected:
    virtual void ImEdit(const sf::Vector2i& mousePixPos) = 0;
    void         setColor(PointId index, sf::Color color) {
                std::size_t i                    = static_cast<std::size_t>(index) * 4;
                entities.pointVerts[i].color     = color;
                entities.pointVerts[i + 1].color = color;
                entities.pointVerts[i + 2].color = color;
                entities.pointVerts[i + 3].color = color;
    }

    void setColor(SpringId index, sf::Color color) {
        std::size_t i                     = static_cast<std::size_t>(index) * 2;
        entities.springVerts[i].color     = color;
        entities.springVerts[i + 1].color = color;
    }

    void resetColor(PointId index) {
        setColor(index, entities.points[static_cast<std::size_t>(index)].color);
    }
    
    void resetColor(SpringId index) {
        setColor(index, sf::Color::White);
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
    void highlightGraph(GraphId i);
    void resetGraphHighlight(GraphId i);
    void DrawGraphs();

    enum class State { normal, newG, editG };

    GraphManager& graphs;
    Graph         defGraph{PointId{}, Property::Position, Component::x, graphs.graphBuffer};
    std::optional<GraphId>  selectedG;
    std::optional<GraphId>  hoveredG;
    std::optional<SpringId> hoveredS;
    std::optional<SpringId> selectedS;
    std::optional<PointId>  hoveredP;
    std::optional<PointId>  selectedP;
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
    void        removePoint(PointId pos);
    void        ImEdit(const sf::Vector2i& mousePixPos) override;
    static void pointInputs(Point& point);

    Point                  defPoint = Point({0.0F, 0.0F}, 1.0F, sf::Color::Red, false);
    std::optional<PointId> hoveredP;
    std::optional<PointId> selectedP;
    double                 toolRange = 1;
    bool                   dragging  = false;
    bool                   inside    = false;
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

    std::vector<Vec2>     verts{{}};
    Polygon               validPoly{};
    std::optional<PolyId> deletingP;
    bool                  isDone      = false;
    bool                  isNewConvex = false;
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
    void        removeSpring(SpringId pos);

    Spring                    defSpring{10, 1.0, 0.2, PointId{}, PointId{}};
    std::array<sf::Vertex, 2> line{sf::Vertex{}, sf::Vertex{}};
    std::optional<SpringId>   selectedS = std::nullopt;
    std::optional<SpringId>   hoveredS  = std::nullopt;
    std::optional<PointId>    selectedP = std::nullopt;
    std::optional<PointId>    hoveredP  = std::nullopt;
    double                    toolRange = 1;
    bool validHover = false; // wether the current hover is an acceptable second point
    bool autoSizing = false;
};