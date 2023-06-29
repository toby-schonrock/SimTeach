#include "Point.hpp"
#include "Polygon.hpp"
#include "Tools.hpp"
#include "ImguiHelpers.hpp"
#include <cstddef>

void PolyTool::frame([[maybe_unused]] Sim& sim, const sf::Vector2i& mousePixPos){
    if (deletingP) {
        entities.polys[static_cast<std::size_t>(*deletingP)].shape.setFillColor(sf::Color::White);
        deletingP.reset();
    }

    sf::Vector2f mousePos = window.mapPixelToCoords(mousePixPos);
    Vec2         newPos   = unvisualize(mousePos);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
        for (PolyId i{}; i != static_cast<PolyId>(entities.polys.size()); ++i) {
            if (entities.polys[static_cast<std::size_t>(i)].isBounded(unvisualize(mousePos)) &&
                entities.polys[static_cast<std::size_t>(i)].isContained(unvisualize(mousePos))) {
                deletingP = i;
            }
        }
        if (deletingP) {
            entities.polys[static_cast<std::size_t>(*deletingP)].shape.setFillColor(sf::Color::Red);
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

void PolyTool::event(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed && !ImGui::GetIO().WantCaptureMouse) {
        if (event.mouseButton.button == sf::Mouse::Left) { // left click
            if (deletingP) {                               // delete
                entities.polys.erase(entities.polys.begin() + static_cast<long long>(*deletingP));
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
void PolyTool::unequip() {
    validPoly = Polygon{};
    verts     = {{}};
    if (deletingP) {
        entities.polys[static_cast<std::size_t>(*deletingP)].shape.setFillColor(sf::Color::White);
        deletingP.reset();
    }
}
