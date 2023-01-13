#pragma once

#include <optional>

#include "Debug.hpp"
#include "GraphMananager.hpp"
#include "RingBuffer.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"
#include "Sim.hpp"
#include "imgui.h"
#include "implot.h"

sf::Vector2f visualize(const Vec2& v);

bool ImGui_DragDouble(const char* label, double* v, float v_speed, double v_min, double v_max,
                      const char* format, ImGuiSliderFlags flags);
bool ImGui_DragUnsigned(const char* label, std::uint32_t* v, float v_speed, std::uint32_t v_min,
                        std::uint32_t v_max, const char* format, ImGuiSliderFlags flags);
void HelpMarker(const char* desc);

class GUI {
  private:
    EntityManager&              entities;
    static constexpr float      zoomFact = 1.05F;
    sf::Texture                 pointTexture;
    sf::RenderWindow&           window;
    std::optional<sf::Vector2i> mousePosLast;
    const Vector2<unsigned int> screen;
    const float                 vsScale; // window scaling
    float                       radius;

  public:
    sf::View         view;
    RingBuffer<Vec2> fps = RingBuffer<Vec2>(160);

    GUI(EntityManager& entities_, const sf::VideoMode& desktop, sf::RenderWindow& window_,
        float radius_ = 0.05F)
        : entities(entities_), window(window_), screen(desktop.width, desktop.height),
          vsScale(static_cast<float>(screen.x) / 20.0F), radius(radius_) {
        std::cout << "Scale: " << vsScale << "\n";
        if (!pointTexture.loadFromFile("point.png"))
            throw std::logic_error("failed to load point texture");
        pointTexture.setSmooth(true);
        reset();
    }

    void reset() {
        view = window.getDefaultView();
        view.zoom(1 / vsScale);
        view.setCenter(view.getSize() / 2.0F);
        window.setView(view);
    }

    void event(const sf::Event& event, const sf::Vector2i& mousePixPos) {
        if (event.type == sf::Event::MouseWheelMoved && !ImGui::GetIO().WantCaptureMouse) {
            float        zoom = (event.mouseWheel.delta == 1) ? 1 / zoomFact : zoomFact;
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

    void frame(const sf::Vector2i& mousePixPos, Sim& sim, GraphManager& graphs) {
        interface(sim, graphs);
        if (sf::Mouse::isButtonPressed(sf::Mouse::Middle)) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll); // make cursor move cursor (was very
                                                               // quick and easy took no time)
            if (!mousePosLast)
                mousePosLast = mousePixPos;
            else {
                sf::Vector2i diff = *mousePosLast - mousePixPos;
                sf::Vector2f mouseMove =
                    sf::Vector2f(diff) * view.getSize().x / static_cast<float>(screen.x);
                view.move(mouseMove);
                window.setView(view);
                mousePosLast = mousePixPos;
            }
        }
    }

    void interface(Sim& sim, GraphManager& graphs) {
        ImGui::Begin("GUI", NULL,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
                         ImGuiWindowFlags_NoResize);
        ImGui::SetWindowSize({-1.0F, -1.0F});
        ImGui::SetWindowPos({0.0F, 0.0F});
        if (ImGui::CollapsingHeader("fps")) {
            ImPlot::PushStyleColor(ImPlotCol_FrameBg, {0, 0, 0, 0});
            ImPlot::PushStyleColor(ImPlotCol_PlotBg, {0, 0, 0, 0});
            if (ImPlot::BeginPlot("fps", {vsScale * 5.0F, vsScale * 2.5F},
                                  ImPlotFlags_NoInputs | ImPlotFlags_NoTitle)) {
                ImPlot::SetupLegend(ImPlotLocation_SouthWest);
                ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoDecorations); // setup axes
                ImPlot::SetupAxis(ImAxis_Y1, "visual");
                ImPlot::SetupAxesLimits(0, 160, 30, 100, ImGuiCond_Always);
                ImPlot::SetupAxis(ImAxis_Y2, "simulation",
                                  ImPlotAxisFlags_Opposite | ImPlotAxisFlags_NoSideSwitch);
                ImPlot::SetupAxisScale(ImAxis_Y2, ImPlotScale_Log10);
                ImPlot::SetupAxisLimits(ImAxis_Y2, 100, 100000);

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

        static bool points   = true;
        static bool springs  = true;
        static bool polygons = true;
        ImGui::Checkbox("Points", &points);
        ImGui::SameLine();
        ImGui::TextDisabled("%zu", entities.points.size());
        ImGui::Checkbox("Springs", &springs);
        ImGui::SameLine();
        ImGui::TextDisabled("%zu", entities.springs.size());
        ImGui::Checkbox("Polgons", &polygons);
        ImGui::SameLine();
        ImGui::TextDisabled("%zu", entities.polys.size());
        if (springs) {
            entities.updateSpringVisPos();
            window.draw(entities.springVerts.data(), entities.springVerts.size(), sf::Lines);
        }
        if (points) {
            ImGui::SetNextItemWidth(100.0F);
            ImGui::DragFloat("Point Radius", &radius, 0.001F, 0.005F, 100000, "%.3f",
                             ImGuiSliderFlags_AlwaysClamp);
            entities.updatePointVisPos(radius);
            window.draw(entities.pointVerts.data(), entities.pointVerts.size(), sf::Quads,
                        &pointTexture);
        }
        if (polygons) {
            for (Polygon& poly: entities.polys) poly.draw(window, false);
        }
        ImGui::SetNextItemWidth(100.0F);
        ImGui_DragDouble("Gravity", &sim.gravity, 0.001F, -100, 100, "%.3f",
                         ImGuiSliderFlags_AlwaysClamp);
        ImGui::SetNextItemWidth(100.0F);
        static std::uint32_t graphBufferTemp = static_cast<std::uint32_t>(
            graphs.graphBuffer); // TODO maybe does this work with size_t ?
        ImGui_DragUnsigned("Graph buffer", &graphBufferTemp, 1.0F, 100, 20000, "%zu",
                           ImGuiSliderFlags_AlwaysClamp);
        graphs.graphBuffer = graphBufferTemp;
        ImGui::SameLine();
        HelpMarker("Graph data is collected every visual frame. The buffer size determines how "
                   "many visual frames before old data is overwritten. This value cannot be "
                   "changed whilst playing.");
        ImGui::Text("View size: (%F, %F)", view.getSize().x, view.getSize().y);
        ImGui::Text("Pos: (%F, %F)", view.getCenter().x, view.getCenter().y);
        if (ImGui::Button("reset view")) reset();
        ImGui::End();
    }
};