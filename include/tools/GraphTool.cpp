#include "Graph.hpp"
#include "GraphMananager.hpp"
#include "ImguiHelpers.hpp"
#include "Tools.hpp"
#include <cstddef>

void GraphTool::highlightGraph(GraphId i) {
    Graph& g = entities.graphs[static_cast<std::size_t>(i)];
    if (g.type == ObjectType::Point) {
        setColor(g.ref.p, selectedPColour);
        if (g.diff == DiffState::Index) setColor(g.ref2.p, selectedPColour);
    } else {
        setColor(g.ref.s, selectedSColour);
        if (g.diff == DiffState::Index) setColor(g.ref2.s, selectedSColour);
    }
}

void GraphTool::resetGraphHighlight(GraphId i) {
    Graph& g = entities.graphs[static_cast<std::size_t>(i)];
    if (g.type == ObjectType::Point) {
        resetColor(g.ref.p);
        if (g.diff == DiffState::Index) resetColor(g.ref2.p);
    } else {
        resetColor(g.ref.s);
        if (g.diff == DiffState::Index) resetColor(g.ref2.s);
    }
}

void GraphTool::DrawGraphs() {
    std::optional<GraphId> newHover = std::nullopt;
    ImGui::Begin("Graphs");
    if (!ImGui::IsWindowCollapsed()) {
        for (GraphId i{}; i != static_cast<GraphId>(entities.graphs.size()); ++i) {
            if (selectedG && i == *selectedG)
                ImPlot::PushStyleColor(ImPlotCol_PlotBg, {0.0F, 1.0F, 0.537F, 0.27F});
            else if (hoveredG && i == *hoveredG)
                ImPlot::PushStyleColor(ImPlotCol_PlotBg, {0.133F, 0.114F, 0.282F, 0.2F});
            entities.graphs[static_cast<std::size_t>(i)].draw(i, graphs.tValues);
            if (ImGui::IsItemHovered()) newHover = static_cast<GraphId>(i);
            if ((hoveredG && i == *hoveredG) || (selectedG && i == *selectedG))
                ImPlot::PopStyleColor();
        }
    }
    hoveredG = newHover;
    ImGui::End();
}

void GraphTool::frame(Sim& sim, const sf::Vector2i& mousePixPos) {
    if (hoveredP) { // color resets
        resetColor(*hoveredP);
        hoveredP.reset();
    }
    if (hoveredS) {
        setColor(*hoveredS, sf::Color::White);
        hoveredS.reset();
    }

    if (!selectedG)
    // TODO graph editing
    {
        // Object hovering
        sf::Vector2f mousePos = window.mapPixelToCoords(mousePixPos);

        if (defGraph.type == ObjectType::Point && !entities.points.empty()) {
            auto [closestPoint, closestPDist] = sim.findClosestPoint(unvisualize(mousePos));
            // color close point for selection
            hoveredP = closestPoint;
            setColor(*hoveredP, hoverPColour);
        } else if (defGraph.type == ObjectType::Spring && !entities.springs.empty()) {
            auto [closestSpring, closestSDist] = sim.findClosestSpring(unvisualize(mousePos));
            // color close spring for selection
            hoveredS = closestSpring;
            setColor(*hoveredS, hoverSColour);
        }
    }
    DrawGraphs();
}

void GraphTool::event(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            if (hoveredG) {                                     // new graph selected
                if (selectedG) resetGraphHighlight(*selectedG); // reset old graph
                selectedG = *hoveredG;
                highlightGraph(*selectedG);
                return;
            }
            if (!ImGui::GetIO().WantCaptureMouse) { // imgui does not want mouse beyond here
                if (selectedG) {                    // deselect graph
                    resetGraphHighlight(*selectedG);
                    selectedG.reset();
                }
                if (defGraph.type == ObjectType::Point && hoveredP) { // point selected
                    if (defGraph.diff != DiffState::Index) {
                        defGraph.ref.p = *hoveredP;
                        entities.graphs.push_back(defGraph);
                    }                                                         // TODO index diff
                } else if (defGraph.type == ObjectType::Spring && hoveredS) { // spring selected
                    if (defGraph.diff != DiffState::Index) {
                        defGraph.ref.s = *hoveredS;
                        entities.graphs.push_back(defGraph);
                    } // TODO index diff
                }
            }
        }
    }
}

void GraphTool::unequip() {
    if (hoveredG) {
        resetGraphHighlight(*hoveredG);
        hoveredG.reset();
    }
    if (selectedG) {
        resetGraphHighlight(*selectedG);
        selectedG.reset();
    }
    if (hoveredP) {
        resetColor(*hoveredP);
        hoveredP.reset();
    }
    if (hoveredS) {
        setColor(*hoveredS, sf::Color::White);
        hoveredS.reset();
    }
}

void GraphTool::ImTool() {
    // graph data dumping
    if (graphs.hasDumped || entities.graphs.empty()) ImGui::BeginDisabled();
    if (ImGui::Button("Save data")) {
        graphs.dumpData();
    } else if (graphs.hasDumped || entities.graphs.empty())
        ImGui::EndDisabled(); // else if to prevent hasdumped change calling enddisabled

    // New graph properties
    if (ImGui::CollapsingHeader("New graph properties",
                                ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow)) {
        int t = static_cast<int>(defGraph.type);
        ImGui::Combo("Type", &t, ObjTypeLbl.data(), ObjTypeLbl.size());
        defGraph.type = static_cast<ObjectType>(t);

        int p;
        if (defGraph.type == ObjectType::Point) {
            p = static_cast<int>(defGraph.prop);
            if (p > 1) p = 0;
            ImGui::Combo("Property", &p, PropLbl.data(), 2);
        } else {
            p = static_cast<int>(defGraph.prop) - 2;
            if (p < 0) p = 0;
            ImGui::Combo("Property", &p, PropLbl.data() + 2,
                         3); // offset for spring properties
            p += 2;
        }
        defGraph.prop = static_cast<Property>(p);

        int c = static_cast<int>(defGraph.comp);
        ImGui::Combo("Component", &c, CompLbl.data(), CompLbl.size());
        defGraph.comp = static_cast<Component>(c);

        int d = static_cast<int>(defGraph.diff);
        ImGui::Combo("Difference", &d, DiffStatLbl.data(), DiffStatLbl.size());
        defGraph.diff = static_cast<DiffState>(d);

        if (defGraph.diff == DiffState::Const) {
            ImGui::InputFloat2("Const Value", &defGraph.constDiff.x);
        }

        std::string type     = ObjTypeLbl[static_cast<std::size_t>(defGraph.type)];
        std::string property = PropLbl[static_cast<std::size_t>(defGraph.prop)];
        std::string comp     = CompLbl[static_cast<std::size_t>(defGraph.comp)];
        std::string formula  = type + "." + property;

        if (defGraph.diff == DiffState::Const)
            formula = "(" + type + "." + property + " - " + defGraph.constDiff.toString() + ")";
        else if (defGraph.diff == DiffState::Index)
            formula = "(" + type + "1." + property + " - " + type + "2." + property + ")";
        ImGui::TextColored(ImVec4{1, 0, 1, 1}, "Formula");
        ImGui::SameLine();
        HelpMarker((formula + "." + comp).c_str());
    }
    if (!selectedG) ImGui::EndDisabled();
}