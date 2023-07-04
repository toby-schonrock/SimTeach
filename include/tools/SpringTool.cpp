#include "ImguiHelpers.hpp"
#include "Tools.hpp"
#include "physics-ency/Spring.hpp"
#include <cstddef>

void SpringTool::ImEdit([[maybe_unused]] const sf::Vector2i& mousePixPos) {
    Spring& spring    = entities.springs[static_cast<std::size_t>(*selectedS)];
    Vec2    springPos = (entities.points[static_cast<std::size_t>(spring.p1)].pos +
                      entities.points[static_cast<std::size_t>(spring.p2)].pos) /
                     2;
    sf::Vector2i springPixPos = window.mapCoordsToPixel(visualize(springPos));
    ImGui::SetNextWindowPos(
        {static_cast<float>(springPixPos.x) + 10.0F, static_cast<float>(springPixPos.y) + 10.0F},
        ImGuiCond_Always);
    ImGui::Begin("Edit Spring", NULL, editFlags);
    ImGui::SetWindowSize({-1.0F, -1.0F}, ImGuiCond_Always);
    springInputs(spring);
    ImGui::SetNextItemWidth(100.0F);
    ImGui::InputDouble("Natural length", &spring.naturalLength, 0, 0, "%.3f");

    // set as tools settings
    if (ImGui::Button("Set as default")) {
        defSpring = entities.springs[static_cast<std::size_t>(*selectedS)];
    }
    ImGui::SameLine();
    HelpMarker("Copys settings to the tool");

    // delete
    if (ImGui::Button("Delete")) {
        removeSpring(*selectedS);
    }
    ImGui::SameLine();
    HelpMarker("LControl + LClick or Delete");
    ImGui::End();
}

void SpringTool::setLineColor(std::array<sf::Vertex, 2>& l, const sf::Color& c) {
    l[0].color = c;
    l[1].color = c;
}

void SpringTool::springInputs(Spring& spring) const {
    ImGui::SetNextItemWidth(100.0F);
    ImGui::InputDouble("Spring constant", &spring.springConst, 0, 0, "%.3f");
    ImGui::SetNextItemWidth(100.0F);
    ImGui::InputDouble("Damping factor", &spring.dampFact, 0, 0, "%.3f");
}

void SpringTool::removeSpring(SpringId pos) {
    entities.rmvSpring(pos);
    if (*selectedS == pos) selectedS.reset();
    if (*hoveredS == pos) hoveredS.reset();
}

void SpringTool::frame(Sim& sim, const sf::Vector2i& mousePixPos) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
        ImGui::SetTooltip("Click to delete");
    }
    if (entities.points.size() == 0) return; // tool is useless if there are no points

    // hover stuff
    if (hoveredP) {
        resetColor(*hoveredP); // reset last closest point color as it may
        hoveredP.reset();      // not be closest anymore
    }
    if (hoveredS) {
        setColor(                         // reset last closest line color as it may  not be
            *hoveredS, sf::Color::White); // closest anymore
        hoveredS.reset();
    }

    if (selectedS) { // if in spring editing mode
        ImEdit(mousePixPos);
    } else { // if in normal mode
        sf::Vector2f mousePos = window.mapPixelToCoords(mousePixPos);
        // determine new closest point (needs to happend wether adding or not)
        auto [closestPoint, closestPDist] = sim.findClosestPoint(unvisualize(mousePos));
        // color close point for selection
        if (closestPDist < toolRange &&
            (!selectedP ||
             *selectedP !=
                 closestPoint)) { // if (in range) and (not selected or the selected != closest)
            hoveredP = closestPoint;
            setColor(*hoveredP, hoverPColour);
        }

        if (selectedP) { // if selected point (in making spring mode)
            line[0].position = visualize(entities.points[static_cast<std::size_t>(*selectedP)].pos);
            if (hoveredP) {
                auto existingS = std::find_if(
                    entities.springs.begin(),
                    entities.springs.end(), // check if spring already exists
                    [hp = *hoveredP, sp = *selectedP](const Spring& s) {
                        return (s.p1 == hp && s.p2 == sp) || (s.p1 == sp && s.p2 == hp);
                    });

                line[1].position =
                    visualize(entities.points[static_cast<std::size_t>(*hoveredP)].pos);
                if (existingS == entities.springs.end()) {
                    setLineColor(line, sf::Color::Green);
                    validHover = true;
                } else {
                    ImGui::SetTooltip("Spring already exists");
                    setLineColor(line, sf::Color::Red);
                    SpringId existingIndex{
                        static_cast<std::size_t>(existingS - entities.springs.begin())};
                    setColor(existingIndex, sf::Color::Red);
                    hoveredS   = existingIndex;
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
                auto [closestSpring, closestSDist] = sim.findClosestSpring(unvisualize(mousePos));
                if (closestSDist < toolRange) {
                    hoveredS = closestSpring;
                    setColor(*hoveredS, hoverSColour);
                }
            }
        }
    }
}

void SpringTool::event(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed && !ImGui::GetIO().WantCaptureMouse) {
        if (selectedS) {
            setColor(*selectedS, sf::Color::White);
            selectedS.reset(); // unselect (close edit)
        } else if (selectedP && event.mouseButton.button !=
                                    sf::Mouse::Left) { // if not a left click unselect point
            resetColor(*selectedP);
            selectedP.reset();
        } else if (event.mouseButton.button == sf::Mouse::Left) {
            if (selectedP) {
                if (validHover) { // if the hover is valid make new spring
                    entities.addSpring(defSpring);
                    Spring& newS = entities.springs.back();
                    newS.p1      = *selectedP;
                    newS.p2      = *hoveredP;
                    if (autoSizing)
                        newS.naturalLength =
                            (entities.points[static_cast<std::size_t>(newS.p1)].pos -
                             entities.points[static_cast<std::size_t>(newS.p2)].pos)
                                .mag();
                    resetColor(*selectedP); // hovered will be reset anyway
                    selectedP.reset();
                }
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) { // fast delete
                if (hoveredS) removeSpring(*hoveredS); // only do it if there is a highlighted
            } else if (hoveredP) {
                selectedP = *hoveredP;
                setColor(*selectedP, selectedPColour);
                hoveredP.reset();
            }
        } else if (event.mouseButton.button == sf::Mouse::Right) { // selecting a spring
            if (hoveredS) {
                selectedS = hoveredS;
                hoveredS.reset();
                setColor(*selectedS, selectedSColour);
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

void SpringTool::unequip() {
    if (selectedP) {
        resetColor(*selectedP);
        selectedP.reset();
    }
    if (hoveredP) {
        resetColor(*hoveredP);
        hoveredP.reset();
    }
    if (selectedS) {
        setColor(*selectedS, sf::Color::White);
        selectedS.reset();
    }
    if (hoveredS) {
        setColor(*hoveredS, sf::Color::White);
        hoveredS.reset();
    }
}

void SpringTool::ImTool() {
    ImGui::SetNextItemWidth(width);
    ImGui_DragDouble("Range", &toolRange, 0.1F, 0.1, 100.0, "%.1f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::Checkbox("Auto sizing", &autoSizing);
    ImGui::SameLine();
    HelpMarker("Springs natural length will be auto set to the distance between the two points");
    if (ImGui::CollapsingHeader(
            "New spring properties",
            ImGuiTreeNodeFlags_DefaultOpen |
                ImGuiTreeNodeFlags_OpenOnArrow)) { // open on arrow to stop insta close bug
        springInputs(defSpring);
        if (autoSizing) ImGui::BeginDisabled();
        ImGui::SetNextItemWidth(100.0F);
        ImGui::InputDouble("Natural length", &defSpring.naturalLength, 0, 0, "%.3f");
        if (autoSizing) ImGui::EndDisabled();
    }
}