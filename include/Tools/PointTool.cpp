#include "Tools.hpp"
#include "ImguiHelpers.hpp"

void PointTool::removePoint(const std::size_t& pos) {
    entities.rmvPoint(pos);
    if (*selectedP == pos) selectedP.reset();
    if (*hoveredP == pos) hoveredP.reset();
}

void PointTool::ImEdit(const sf::Vector2i& mousePixPos) {
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
    ImGui::Begin("Edit Point", NULL,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    ImGui::SetWindowSize({-1.0F, -1.0F}, ImGuiCond_Always);

    // properties
    ImGui::SetNextItemWidth(width);
    Vec2F posTemp = Vec2F(point.pos);
    ImGui::DragFloat2("Position", &posTemp.x, 0.01F);
    point.pos = Vec2(posTemp);
    ImGui::SameLine();
    if (ImGui::Button("Drag")) {
        dragging = true;
        sf::Mouse::setPosition(pointPixPos, window);
    }

    pointInputs(point);

    // set as tools settings
    if (ImGui::Button("Set as default")) {
        defPoint = entities.points[*selectedP];
    }
    ImGui::SameLine();
    HelpMarker("Copy settings to the spring tool");

    // delete
    if (ImGui::Button("Delete")) {
        removePoint(*selectedP);
    }
    ImGui::SameLine();
    HelpMarker("LControl + LClick or Delete");
    ImGui::End();
}

void PointTool::pointInputs(Point& point) {
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
    if (ImGui::Button("Reset")) {
        point.vel = Vec2();
    }

    ImGui::SetNextItemWidth(width);
    ImGui_DragDouble("Mass", &(point.mass), 0.1F, 0.1, 100.0, "%.1f", ImGuiSliderFlags_AlwaysClamp);

    if (point.fixed) {
        ImGui::EndDisabled();
    }

    // akward colour translation stuff // remember point.color does not draw that color (needs a
    // Point::updateColor())
    float imcol[4] = {
        static_cast<float>(point.color.r) / 255, static_cast<float>(point.color.g) / 255,
        static_cast<float>(point.color.b) / 255, static_cast<float>(point.color.a) / 255};
    ImGui::ColorPicker4("Color", imcol);
    point.color =
        sf::Color(static_cast<uint8_t>(imcol[0] * 255.0F), static_cast<uint8_t>(imcol[1] * 255.0F),
                  static_cast<uint8_t>(imcol[2] * 255.0F), static_cast<uint8_t>(imcol[3] * 255.0F));
}

void PointTool::frame(Sim& sim, const sf::Vector2i& mousePixPos) {
    Vec2 mousePos = unvisualize(window.mapPixelToCoords(mousePixPos));
    auto poly =
        std::find_if(entities.polys.begin(), entities.polys.end(), [mousePos](const Polygon& p) {
            return p.isBounded(mousePos) && p.isContained(mousePos);
        });

    inside = poly != entities.polys.end();

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

void PointTool::event(const sf::Event& event) {
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
                defPoint.pos = unvisualize(
                    window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y}));
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

void PointTool::unequip() {
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

void PointTool::ImTool() {
    ImGui::SetNextItemWidth(width);
    ImGui_DragDouble("Range", &toolRange, 0.1F, 0.1, 100.0, "%.1f", ImGuiSliderFlags_AlwaysClamp);
    if (ImGui::CollapsingHeader(
            "New point properties",
            ImGuiTreeNodeFlags_DefaultOpen |
                ImGuiTreeNodeFlags_OpenOnArrow)) { // open on arrow to stop insta close bug
        pointInputs(defPoint);
    }
}