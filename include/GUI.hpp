#pragma once

#include <optional>

#include "RingBuffer.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"
#include "Sim.hpp"
#include "Vector2.hpp"
#include "imgui.h"
#include "implot.h"

sf::Vector2f visualize(const Vec2& v);

class GUI {
  private:
    sf::RenderWindow&           window;
    std::optional<sf::Vector2i> mousePosLast;
    const Vector2<unsigned int> screen;
    const float                 vsScale; // window scaling
  public:
    sf::View         view;
    RingBuffer<Vec2> fps = RingBuffer<Vec2>(160);

    GUI(const sf::VideoMode& desktop, sf::RenderWindow& window_)
        : window(window_), screen(desktop.width, desktop.height),
          vsScale(static_cast<float>(screen.x) / 20.0F) {
        std::cout << "Scale: " << vsScale << "\n";
        reset();
    }

    void reset() {
        view = window.getDefaultView();
        view.zoom(1 / vsScale);
        view.setCenter(view.getSize() / 2.0F);
        window.setView(view);
    }

    void event(const sf::Event& event, const sf::Vector2i& mousePixPos) {
        if (event.type == sf::Event::MouseWheelMoved) {
            float        zoom = (event.mouseWheel.delta == 1) ? 1 / 1.05F : 1.05F;
            sf::Vector2f diff = window.mapPixelToCoords(mousePixPos) - view.getCenter();
            view.zoom(zoom);
            view.move(diff * (1 - zoom));
            window.setView(view);
        } else if (event.type == sf::Event::MouseButtonReleased) {
            if (event.mouseButton.button == sf::Mouse::Middle) {
                mousePosLast.reset();
            }
        }
    }

    void frame(Sim& sim, const sf::Vector2i& mousePos) {
        interface(sim);
        if (sf::Mouse::isButtonPressed(sf::Mouse::Middle)) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll); // make cursor move cursor (was very
                                                               // quick and easy took no time)
            if (!mousePosLast)
                mousePosLast = mousePos;
            else {
                sf::Vector2i diff = *mousePosLast - mousePos;
                sf::Vector2f mouseMove =
                    sf::Vector2f(diff) * view.getSize().x / static_cast<float>(screen.x);
                view.move(mouseMove);
                window.setView(view);
                mousePosLast = mousePos;
            }
        }
    }

    void interface(Sim& sim) {
        ImGui::Begin("GUI", NULL,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
                         ImGuiWindowFlags_NoResize);
        ImGui::SetWindowSize({-1.0F, -1.0F});
        ImGui::SetWindowPos({0.0F, 0.0F});
        if (ImGui::CollapsingHeader("fps")) {
            ImPlot::PushStyleColor(ImPlotCol_FrameBg, {0, 0, 0, 0});
            ImPlot::PushStyleColor(ImPlotCol_PlotBg, {0, 0, 0, 0});
            if (ImPlot::BeginPlot(
                    "fps", {vsScale * 5.0F, vsScale * 2.5F},
                    ImPlotFlags_NoInputs |
                        ImPlotFlags_NoTitle)) { // NOLINT "Use of a signed integer operand with a
                                                // binary bitwise operator" this is implots fault
                ImPlot::SetupLegend(ImPlotLocation_SouthWest);
                ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoDecorations); // setup axes
                ImPlot::SetupAxis(ImAxis_Y1, "visual");
                ImPlot::SetupAxesLimits(0, 160, 50, 100, ImGuiCond_Always);
                ImPlot::SetupAxis(ImAxis_Y2, "simulation",
                                  ImPlotAxisFlags_Opposite | ImPlotAxisFlags_NoSideSwitch);
                ImPlot::SetupAxisScale(ImAxis_Y2, ImPlotScale_Log10);
                ImPlot::SetupAxisLimits(ImAxis_Y2, 1000, 100000);

                ImPlot::PlotLine("visual", &fps.v[0].x, static_cast<int>(fps.v.size()), 1.0L, 0.0L,
                                 ImPlotLineFlags_None, static_cast<int>(fps.pos),
                                 sizeof(Vec2)); // plot graphs
                ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
                ImPlot::PlotLine("simulation", &fps.v[0].y, static_cast<int>(fps.v.size()), 1, 0.0L,
                                 ImPlotLineFlags_None, static_cast<int>(fps.pos), sizeof(Vec2));
                ImPlot::EndPlot();
            }
            ImPlot::PopStyleColor(2);
        }

        static bool springs  = true;
        static bool polygons = true;
        static bool points   = true;
        ImGui::Checkbox("Springs", &springs);
        ImGui::Checkbox("Polgons", &polygons);
        ImGui::Checkbox("Points", &points);
        if (springs) {
            for (Spring& spring: sim.springs) {
                spring.verts[0].position = visualize(sim.points[spring.p1].pos);
                spring.verts[1].position = visualize(sim.points[spring.p2].pos);
                window.draw(spring.verts.data(), 2, sf::Lines);
            }
        }
        if (polygons) {
            for (Polygon& poly: sim.polys) poly.draw(window);
        }
        if (points) {
            for (Point& point: sim.points) point.draw(window);
        }

        ImGui::Text("View: (%F, %F)", view.getSize().x, view.getSize().y);
        ImGui::Text("Pos: (%F, %F)", view.getCenter().x, view.getCenter().y);
        if (ImGui::Button("reset view")) reset();
        ImGui::End();
    }
};