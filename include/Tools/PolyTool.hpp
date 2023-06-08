#pragma once

#include "Tool.hpp"
#include "Polygon.hpp"

class PolyTool : public Tool {
  private:
    std::vector<Vec2>          verts{{}};
    Polygon                    validPoly{};
    std::optional<std::size_t> deletingP;
    bool                       isDone      = false;
    bool                       isNewConvex = false;

    void ImEdit([[maybe_unused]] const sf::Vector2i& mousePixPos) override {}

  public:
    PolyTool(sf::RenderWindow& window_, EntityManager& entities_, const std::string& name_)
        : Tool(window_, entities_, name_) {}

    void frame([[maybe_unused]] Sim& sim, const sf::Vector2i& mousePixPos) override {
        if (deletingP) {
            entities.polys[*deletingP].shape.setFillColor(sf::Color::White);
            deletingP.reset();
        }

        sf::Vector2f mousePos = window.mapPixelToCoords(mousePixPos);
        Vec2         newPos   = unvisualize(mousePos);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
            for (std::size_t i = 0; i != entities.polys.size(); ++i) {
                if (entities.polys[i].isBounded(unvisualize(mousePos)) &&
                    entities.polys[i].isContained(unvisualize(mousePos))) {
                    deletingP = i;
                }
            }
            if (deletingP) {
                entities.polys[*deletingP].shape.setFillColor(sf::Color::Red);
                ImGui::SetTooltip("Click to delete");
                return;
            }
        }

        verts.back() = newPos;
        isDone       = false;
        isNewConvex  = false;
        if (verts.size() >= 2) {
            Polygon newPoly = Polygon{verts};
            isNewConvex     = newPoly.isConvex();
            if (verts.size() >= 3) {
                verts.pop_back();
                validPoly = Polygon{verts};
                verts.push_back(newPos);
                isDone = validPoly.edges.size() >= 3 && validPoly.isContained(newPos);
            }
            if (isDone) { // valid drawn
                validPoly.shape.setFillColor(sf::Color::Green);
                validPoly.draw(window, true);
            } else { // new drawn
                newPoly.shape.setFillColor(isNewConvex ? sf::Color::White : sf::Color::Red);
                newPoly.draw(window, true);
            }
        }
    }

    void event(const sf::Event& event) override {
        if (event.type == sf::Event::MouseButtonPressed && !ImGui::GetIO().WantCaptureMouse) {
            if (event.mouseButton.button == sf::Mouse::Left) { // left click
                if (deletingP) {                               // delete
                    entities.polys.erase(entities.polys.begin() +
                                         static_cast<long long>(*deletingP));
                    deletingP.reset();
                } else if (isDone) { // if green
                    validPoly.shape.setFillColor(sf::Color::White);
                    validPoly.boundsUp();
                    validPoly.isConvex(); // not sure if nessecary
                    entities.polys.push_back(validPoly);
                    verts     = {{}};
                    validPoly = Polygon{};
                } else if (verts.size() <= 3 || isNewConvex) {
                    verts.push_back({});
                }
            } else if (event.mouseButton.button == sf::Mouse::Right) {
                if (verts.size() != 1) verts.pop_back();
            }
        }
    }
    void unequip() override {
        validPoly = Polygon{};
        verts     = {{}};
        if (deletingP) {
            entities.polys[*deletingP].shape.setFillColor(sf::Color::White);
            deletingP.reset();
        }
    }
    void ImTool() override {}
};