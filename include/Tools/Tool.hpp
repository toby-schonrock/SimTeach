#pragma once

#include "ImguiHelpers.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"
#include "Sim.hpp"
#include "Spring.hpp"
#include "Fundamentals/Vector2.hpp"
#include "imgui.h"
#include <cstddef>
#include <iostream>
#include <optional>

static inline const sf::Color selectedPColour = sf::Color::Magenta;
static inline const sf::Color hoverPColour    = sf::Color::Blue;
static inline const sf::Color selectedSColour = sf::Color::Magenta;
static inline const sf::Color hoverSColour    = sf::Color::Blue;

static inline const float width = 120;

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