#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <numbers>
#include <string>

#include "Matrix.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "RingBuffer.hpp"
#include "Vector2.hpp"
#include "imgui-SFML.h"
#include "imgui.h"
#include "implot.h"

float                     vsScale = 0;
extern unsigned char      arial_ttf[]; // NOLINT
extern const unsigned int arial_ttf_len;

class SoftBody {
  public:
    Vec2I size;
    Vec2  simPos;
    float springConst = 9000;
    float dampFact    = 100;
    float gap;

  private:
    Matrix<Point>          points;
    static constexpr float radius = 0.05F;

  public:
    SoftBody(const Vec2I& size_, float gap_, const Vec2& simPos_, float springConst_,
             float dampFact_)
        : size(size_), simPos(simPos_), springConst(springConst_), dampFact(dampFact_), gap(gap_),
          points(size.x, size.y) {
        for (int x = 0; x < size.x; x++) {
            for (int y = 0; y < size.y; y++) {
                points(x, y) = Point(Vec2(x, y) * gap + simPos, 1.0, radius);
            }
        }
    }

    void reset() { // evil function
        *this = SoftBody(size, gap, simPos, springConst, dampFact);
    }

    void draw(sf::RenderWindow& window) {
        for (Point& point: points.v) point.draw(window);
    }

    void simFrame(double deltaTime, double gravity, const std::vector<Polygon>& polys) {
        for (int x = 0; x < points.sizeX; x++) {
            for (int y = 0; y < points.sizeY; y++) {
                Point& p = points(x, y);
                if (x < points.sizeX - 1) {
                    if (y < points.sizeY - 1) {
                        Point::springHandler(p, points(x + 1, y + 1), std::numbers::sqrt2 * gap,
                                             springConst, dampFact); // down right
                    }
                    Point::springHandler(p, points(x + 1, y), gap, springConst, dampFact); // right
                }
                if (y < points.sizeY - 1) {
                    if (x > 0) {
                        Point::springHandler(p, points(x - 1, y + 1), std::numbers::sqrt2 * gap,
                                             springConst, dampFact); // down left
                    }
                    Point::springHandler(p, points(x, y + 1), gap, springConst, dampFact); // down
                }
            }
        }
        for (Point& point: points.v) {
            point.update(deltaTime, gravity);
        }

        for (const Polygon& poly: polys) {
            for (Point& point: points.v) {
                if (poly.isBounded(point.pos)) point.polyColHandler(poly);
            }
        }
    }
};

sf::Vector2f visualize(const Vec2& v) {
    return sf::Vector2f(static_cast<float>(v.x), static_cast<float>(v.y)) * vsScale;
}

void displayFps(const RingBuffer<Vec2>& fps) {
    ImGui::Begin("FPS");
    if (ImPlot::BeginPlot(
            "fps", {-1.0F, -1.0F},
            ImPlotFlags_NoInputs |
                ImPlotFlags_NoTitle)) { // NOLINT "Use of a signed integer operand with a binary
                                        // bitwise operator" this is implots fault
        ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoDecorations); // setup axes
        ImPlot::SetupAxis(ImAxis_Y1, "visual");
        ImPlot::SetupAxesLimits(0, 160, 0, 100, ImGuiCond_Always);
        ImPlot::SetupAxis(ImAxis_Y2, "simulation",
                          ImPlotAxisFlags_Opposite | ImPlotAxisFlags_NoSideSwitch);
        ImPlot::SetupAxisScale(ImAxis_Y2, ImPlotScale_Log10);
        ImPlot::SetupAxisLimits(ImAxis_Y2, 1, 100000);

        ImPlot::PlotLine("visual", &fps.v[0].x, static_cast<int>(fps.v.size()), 1.0L, 0.0L,
                         ImPlotLineFlags_None, static_cast<int>(fps.pos),
                         sizeof(Vec2)); // plot graphs
        ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
        ImPlot::PlotLine("simulation", &fps.v[0].y, static_cast<int>(fps.v.size()), 1, 0.0L,
                         ImPlotLineFlags_None, static_cast<int>(fps.pos), sizeof(Vec2));
        ImPlot::EndPlot();
    }
    ImGui::End();
}

void displaySimSettings(SoftBody& sb, float& gravity) {
    ImGui::Begin("Settings");
    ImGui::DragFloat("Gravity", &gravity, 0.01F);
    ImGui::DragFloat("Gap", &sb.gap, 0.005F);
    ImGui::DragFloat("Spring Constant", &sb.springConst, 10.0F, 0.0F, 20000.0F);
    ImGui::DragFloat("Damping Factor", &sb.dampFact, 1.0F, 0.0F, 300.0F);
    ImGui::DragInt("Size X", &sb.size.x, 1, 2, 50);
    ImGui::DragInt("Size Y", &sb.size.y, 1, 2, 50);
    ImGui::DragFloat("Zoom", &vsScale, 1, 0, 250);
    if (ImGui::Button("Reset sim")) sb.reset();
    ImGui::SameLine();
    if (ImGui::Button("Default sim")) {
        sb      = SoftBody(Vec2I(25, 25), 0.2F, Vec2(3, 0), 8000, 100);
        gravity = 2.0F;
    }
    ImGui::End();
}

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

    SoftBody sb(Vec2I(25, 25), 0.2F, Vec2(3, 0), 10000, 100);
    float            gravity = 2;

    std::vector<Polygon> polys;
    polys.push_back(Polygon::Square(Vec2(6, 10), -0.75));
    polys.push_back(Polygon::Square(Vec2(14, 10), 0.75));
    polys.push_back(Polygon::Triangle(Vec2(100, 100)));

    RingBuffer<Vec2> fps(160);
    double                                     Vfps = 0;
    double                                     Sfps = 0;

    std::chrono::_V2::system_clock::time_point last = std::chrono::high_resolution_clock::now(); // setting time of previous frame to be now
    sf::Clock
        deltaClock; // for imgui - read https://eliasdaler.github.io/using-imgui-with-sfml-pt1/
    while (window.isOpen()) {
        std::chrono::_V2::system_clock::time_point start =
            std::chrono::high_resolution_clock::now();

        // clear poll events for sfml and imgui
        sf::Event event; // NOLINT
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed) window.close();
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        int                      simFrames   = 0;
        std::chrono::nanoseconds sinceVFrame = std::chrono::high_resolution_clock::now() - start;
        while ((sinceVFrame.count() < 10'000'000)) { // TODO: min max avg frames test
            ++simFrames;
            std::chrono::_V2::system_clock::time_point newLast =
                std::chrono::high_resolution_clock::now();
            constexpr std::chrono::nanoseconds maxFrame{1'000'000};
            std::chrono::nanoseconds           deltaTime = std::min(newLast - last, maxFrame);
            last                                         = newLast;

            sb.simFrame(static_cast<double>(deltaTime.count()) / 1e9, gravity, polys);
            sinceVFrame = std::chrono::high_resolution_clock::now() - start;
        }

        // draw

        window.clear();

        //imgui windows
        displaySimSettings(sb, gravity);
        displayFps(fps);

        sb.draw(window);
        for (Polygon& poly: polys) poly.draw(window);

        ImGui::SFML::Render(window);
        window.display();

        sinceVFrame = std::chrono::high_resolution_clock::now() - start;
        Sfps = 1e9 * simFrames / static_cast<double>(sinceVFrame.count());
        Vfps = 1e9 / static_cast<double>(sinceVFrame.count());
        fps.add({Vfps, Sfps});
    }

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();

    return 0;
}