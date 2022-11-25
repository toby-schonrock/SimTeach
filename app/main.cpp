#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numbers>
#include <optional>

#include "Matrix.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "RingBuffer.hpp"
#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "Sim.hpp"
#include "Vector2.hpp"
#include "imgui-SFML.h"
#include "imgui.h"
#include "implot.h"

const sf::Color  selectedColour = sf::Color::Blue;
constexpr double sliceRange     = 0.1;
constexpr double deleteRange    = 1;
float            vsScale        = 0;

extern unsigned char      arial_ttf[]; // NOLINT
extern const unsigned int arial_ttf_len;

sf::Vector2f visualize(const Vec2& v) {
    return sf::Vector2f(static_cast<float>(v.x), static_cast<float>(v.y)) * vsScale;
}

Vec2 unvisualize(const sf::Vector2f& v) { return Vec2(v.x, v.y) / vsScale; }

Vec2 unvisualize(const sf::Vector2i& v) { return Vec2(v.x, v.y) / vsScale; }

void displayFps(const RingBuffer<Vec2>& fps) {
    ImGui::Begin("FPS", NULL,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNav |
                     ImGuiWindowFlags_NoDecoration);
    ImPlot::PushStyleColor(ImPlotCol_FrameBg, {0, 0, 0, 0});
    ImPlot::PushStyleColor(ImPlotCol_PlotBg, {0, 0, 0, 0});
    if (ImPlot::BeginPlot(
            "fps", {-1.0F, -1.0F},
            ImPlotFlags_NoInputs |
                ImPlotFlags_NoTitle)) { // NOLINT "Use of a signed integer operand with a binary
                                        // bitwise operator" this is implots fault
        ImPlot::SetupLegend(ImPlotLocation_SouthWest);
        ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoDecorations); // setup axes
        ImPlot::SetupAxis(ImAxis_Y1, "visual");
        ImPlot::SetupAxesLimits(0, 160, 50, 100, ImGuiCond_Always);
        ImPlot::SetupAxis(ImAxis_Y2, "simulation",
                          ImPlotAxisFlags_Opposite | ImPlotAxisFlags_NoSideSwitch);
        ImPlot::SetupAxisScale(ImAxis_Y2, ImPlotScale_Log10);
        ImPlot::SetupAxisLimits(ImAxis_Y2, 1000, 50000);

        ImPlot::PlotLine("visual", &fps.v[0].x, static_cast<int>(fps.v.size()), 1.0L, 0.0L,
                         ImPlotLineFlags_None, static_cast<int>(fps.pos),
                         sizeof(Vec2)); // plot graphs
        ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
        ImPlot::PlotLine("simulation", &fps.v[0].y, static_cast<int>(fps.v.size()), 1, 0.0L,
                         ImPlotLineFlags_None, static_cast<int>(fps.pos), sizeof(Vec2));
        ImPlot::EndPlot();
    }
    ImPlot::PopStyleColor(2);

    ImGui::End();
}

// void displaySimSettings(SoftBody& sb, float& gravity) {
//     ImGui::Begin("Settings");
//     ImGui::DragFloat("Gravity", &gravity, 0.01F);
//     ImGui::DragFloat("Gap", &sb.gap, 0.005F);
//     ImGui::DragFloat("Spring Constant", &sb.springConst, 10.0F, 0.0F, 20000.0F);
//     ImGui::DragFloat("Damping Factor", &sb.dampFact, 1.0F, 0.0F, 300.0F);
//     ImGui::DragInt("Size X", &sb.size.x, 1, 2, 50);
//     ImGui::DragInt("Size Y", &sb.size.y, 1, 2, 50);
//     ImGui::DragFloat("Zoom", &vsScale, 1, 0, 250);
//     if (ImGui::Button("Reset sim")) sb.reset();
//     ImGui::SameLine();
//     if (ImGui::Button("Default sim")) {
//         sb      = SoftBody(Vec2I(25, 25), 0.2F, Vec2(3, 0), 8000, 100);
//         gravity = 2.0F;
//     }
//     ImGui::End();
// }

int main() {
    sf::VideoMode               desktop = sf::VideoMode::getDesktopMode();
    const Vector2<unsigned int> screen(desktop.width, desktop.height);
    vsScale = 25.0F / 512.0F * static_cast<float>(screen.x); // window scaling

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Simulation", sf::Style::Fullscreen,
                            settings); //, sf::Style::Default);
    ImGui::SFML::Init(window);
    ImPlot::CreateContext();

    Sim sim1 = Sim::softbody({25, 25}, {5, 0}, 0.05F, 2.0F, 0.2F, 8000, 100);

    RingBuffer<Vec2>           fps(160);
    std::optional<std::size_t> pointLastChanged;

    std::chrono::system_clock::time_point last =
        std::chrono::high_resolution_clock::now(); // setting time of previous frame to be now
    sf::Clock
        deltaClock; // for imgui - read https://eliasdaler.github.io/using-imgui-with-sfml-pt1/
    while (window.isOpen()) {
        std::chrono::system_clock::time_point start = std::chrono::high_resolution_clock::now();

        // closest point malarkey
        if (pointLastChanged) sim1.points[*pointLastChanged].shape.setFillColor(sim1.color);
        Vec2 mousePos                    = unvisualize(sf::Mouse::getPosition(window));
        auto [closestPoint, closestDist] = sim1.findClosestPoint(mousePos);
        if (closestDist < 1) {
            sim1.points[closestPoint].shape.setFillColor(selectedColour);
            pointLastChanged = closestPoint;
        } else {
            pointLastChanged.reset();
        }

        // clear poll events for sfml and imgui
        sf::Event event; // NOLINT
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            switch (event.type) {
            case sf::Event::Closed:
                window.close();
                break;

            case sf::Event::MouseButtonPressed:
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (closestDist < deleteRange) sim1.removePoint(closestPoint);
                    pointLastChanged.reset();
                }
                break;
            default:
                break;
            }
        }
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && closestDist < sliceRange) {
            sim1.removePoint(closestPoint);
            pointLastChanged.reset();
            auto closest = sim1.findClosestPoint(mousePos);
            while (closest.second < sliceRange) {
                sim1.removePoint(closest.first);
                closest = sim1.findClosestPoint(mousePos);
            }
        };

        ImGui::SFML::Update(window, deltaClock.restart());

        int                      simFrames   = 0;
        std::chrono::nanoseconds sinceVFrame = std::chrono::high_resolution_clock::now() - start;
        while ((sinceVFrame.count() < 10'000'000)) { // TODO: min max avg frames test
            ++simFrames;
            std::chrono::system_clock::time_point newLast =
                std::chrono::high_resolution_clock::now();
            constexpr std::chrono::nanoseconds maxFrame{1'000'000};
            std::chrono::nanoseconds           deltaTime = std::min(newLast - last, maxFrame);
            last                                         = newLast;

            sim1.simFrame(static_cast<double>(deltaTime.count()) / 1e9);
            sinceVFrame = std::chrono::high_resolution_clock::now() - start;
        }

        // draw
        window.clear();

        // imgui windows
        //  displaySimSettings(sb, gravity);
        displayFps(fps);

        sim1.draw(window);

        ImGui::SFML::Render(window);
        window.display();

        sinceVFrame       = std::chrono::high_resolution_clock::now() - start;
        const double Sfps = 1e9 * simFrames / static_cast<double>(sinceVFrame.count());
        const double Vfps = 1e9 / static_cast<double>(sinceVFrame.count());
        fps.add({Vfps, Sfps});
    }

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();

    return 0;
}