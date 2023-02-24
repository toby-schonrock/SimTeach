#pragma once

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "Debug.hpp"
#include "EntityManager.hpp"
#include "Graph.hpp"
#include "GraphMananager.hpp"
#include "RingBuffer.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/System/Vector2.hpp"
#include "SFML/Window.hpp"
#include "Sim.hpp"
#include "imgui.h"
#include "implot.h"

namespace fs = std::filesystem;

extern const fs::path Previous;

sf::Vector2f visualize(const Vec2& v);

bool ImGui_DragDouble(const char* label, double* v, float v_speed, double v_min, double v_max,
                      const char* format, ImGuiSliderFlags flags);
bool ImGui_DragUnsigned(const char* label, std::uint32_t* v, float v_speed, std::uint32_t v_min,
                        std::uint32_t v_max, const char* format, ImGuiSliderFlags flags);
void HelpMarker(const char* desc);

inline void enabledCheckBoxes(ObjectEnabled& enabled, const EntityManager& entities,
                         const std::string& uuid, bool forceLegit = false) {
    if (enabled.springs && forceLegit) ImGui::BeginDisabled();
    ImGui::Checkbox((std::string{"Points##"} + uuid).c_str(), &enabled.points);
    ImGui::SameLine();
    ImGui::TextDisabled("%zu", entities.points.size());
    if (enabled.springs && forceLegit) ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Checkbox((std::string{"Springs##"} + uuid).c_str(), &enabled.springs);
    ImGui::SameLine();
    ImGui::TextDisabled("%zu", entities.springs.size());
    ImGui::SameLine();
    ImGui::Checkbox((std::string{"Polygons##"} + uuid).c_str(), &enabled.polygons);
    ImGui::SameLine();
    ImGui::TextDisabled("%zu", entities.polys.size());
    if (enabled.springs && forceLegit) enabled.points = true;
}

// stolen from - https://en.cppreference.com/w/cpp/filesystem/directory_entry/last_write_time
inline std::string to_string(fs::file_time_type const& ftime) {
    std::time_t cftime =
        std::chrono::system_clock::to_time_t(std::chrono::file_clock::to_sys(ftime));
    std::string str = std::asctime(std::localtime(&cftime));
    str.pop_back(); // rm the trailing '\n' put by `asctime`
    return str;
}

inline void display_size(fs::directory_entry file) {
    int    i{};
    double mantissa = static_cast<double>(file.file_size());
    for (; mantissa >= 1024.; ++i) {
        mantissa /= 1024.;
    }
    mantissa = std::ceil(mantissa * 10.) / 10.;
    ImGui::Text("%.4g %cB", mantissa, "BKMGTPE"[i]);
}

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
    ObjectEnabled               loading{true, true, true};
    ObjectEnabled               saving{true, true, true};
    ObjectEnabled               display{true, true, true};

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

    void frame(const sf::Vector2i& mousePixPos, Sim& sim, GraphManager& graphs, bool running) {
        interface(mousePixPos, sim, graphs, running);
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

    void interface(const sf::Vector2i& mousePixPos, Sim& sim, GraphManager& graphs, bool running) {
        ImGui::Begin("Settings", NULL,
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
                         ImGuiWindowFlags_NoResize);

        ImGui::SetWindowSize({-1.0F, -1.0F}, ImGuiCond_Always);
        ImGui::SetWindowPos({0.0F, 0.0F}, ImGuiCond_Once);

        if (ImGui::CollapsingHeader("Save and load")) {
            if (running) ImGui::BeginDisabled();
            static char arr[20]{};
            const auto  invalid =
                std::find_if(&arr[0], &arr[20], [](unsigned char c) { return !std::isalnum(c); });
            const auto end   = std::find(&arr[0], &arr[20], '\0');
            const bool valid = (invalid == &arr[20] || invalid >= end) && end - &arr[0] != 0;
            fs::path   savePath{arr};
            savePath = "sims/" + savePath.string() + ".csv";

            ImGui::BulletText("Saving");
            ImGui::Indent(10.0F);
            enabledCheckBoxes(saving, entities, "saving", true);
            ImGui::SameLine();
            HelpMarker("Enable and disable which items are saved");
            ImGui::InputText("Filename", &arr[0], 20);
            ImGui::SameLine();
            HelpMarker("Filenames which are already in use will be overidden.");
            if (!valid) {
                if (invalid != &arr[20] && invalid < end)
                    ImGui::TextColored(ImVec4{1, 0, 0, 1},
                                       "All characters must be alpha numeric - '%c'", *invalid);
                else if (end - &arr[0] == 0) {
                    ImGui::TextColored(ImVec4{1, 0, 0, 1}, "File name has no characters! :(");
                }
            } else {
                ImGui::Text("Path: %s", savePath.string().c_str());
            }
            if (!valid) ImGui::BeginDisabled();
            if (ImGui::Button("Save")) {
                sim.save(savePath, saving);
            }
            if (!valid) ImGui::EndDisabled();
            ImGui::Unindent(10.0F);

            ImGui::BulletText("Loading");
            ImGui::Indent(10.0F);
            enabledCheckBoxes(loading, entities, "loading", true);
            ImGui::SameLine();
            HelpMarker("Enable and disable which items are loaded");

            static bool overwrite = true;
            ImGui::Checkbox("overwrite", &overwrite);
            ImGui::SameLine();
            HelpMarker("Overrite enabled will delete the current sim before loading the new one");

            std::vector<fs::directory_entry> files;
            for (const auto& entry: fs::directory_iterator(fs::path{"sims"}))
                if (entry.path().extension() == ".csv") files.push_back(entry);
            std::sort(files.begin(), files.end(), [](const auto& lhs, const auto& rhs) {
                return lhs.last_write_time() > rhs.last_write_time();
            });

            static std::size_t current = 0;
            float              height  = ImGui::GetTextLineHeightWithSpacing() *
                               std::min(10.0F, static_cast<float>(files.size())) + 2.0F;
            if (ImGui::BeginListBox("File", {440.0f, height})) {
                for (std::size_t i = 0; i < files.size(); ++i) {
                    const bool is_selected = current == i;
                    if (ImGui::Selectable(files[i].path().stem().string().c_str(), is_selected))
                        current = i;
                    ImGui::SameLine(160.0F);
                    display_size(files[i]);
                    ImGui::SameLine(240.0F);
                    ImGui::Text("%s", to_string(files[i].last_write_time()).c_str());
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndListBox();
            }
            if (ImGui::Button("Load") &&
                current < files.size()) { // prevents old selection breaking the load
                sim.load(files[current].path(), overwrite, loading);
            }
            ImGui::Unindent(10.0F);
            if (running) ImGui::EndDisabled();
        }
        if (ImGui::CollapsingHeader("General")) {
            if (running) ImGui::BeginDisabled();
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
                       "many visual frames occur before old data is overwritten. This value is "
                       "updated "
                       "on run.");
            if (running) ImGui::EndDisabled();
        }

        if (ImGui::CollapsingHeader("Graphics")) {
            fpsGraph();
            enabledCheckBoxes(display, entities, "display");
            ImGui::SameLine();
            HelpMarker("Enable and disable which items are displayed (usefull for laggy scenes)");
            ImGui::SetNextItemWidth(100.0F);
            ImGui::DragFloat("Point Radius", &radius, 0.001F, 0.005F, 100000, "%.3f",
                             ImGuiSliderFlags_AlwaysClamp);
        }

        if (display.springs) {
            entities.updateSpringVisPos();
            window.draw(entities.springVerts.data(), entities.springVerts.size(), sf::Lines);
        }
        if (display.points) {
            entities.updatePointVisPos(radius);
            window.draw(entities.pointVerts.data(), entities.pointVerts.size(), sf::Quads,
                        &pointTexture);
        }
        if (display.polygons) {
            for (Polygon& poly: entities.polys) poly.draw(window, false);
        }

        ImGui::Text("View size: (%F, %F)", view.getSize().x, view.getSize().y);
        ImGui::Text("View center: (%F, %F)", view.getCenter().x, view.getCenter().y);
        sf::Vector2f mousePos = window.mapPixelToCoords(mousePixPos);
        ImGui::Text("Mouse pos: (%F, %F)", mousePos.x, mousePos.y);
        if (ImGui::Button("Reset view")) reset();
        ImGui::SameLine();
        HelpMarker("Resets the view to default");
        if (running) ImGui::BeginDisabled();
        if (ImGui::Button("Reset sim")) sim.reset();
        if (running) ImGui::EndDisabled();
        ImGui::SameLine();
        HelpMarker("Resets the sim to last starting point - r");
        ImGui::End();
    }

    void fpsGraph() {
        ImPlot::PushStyleColor(ImPlotCol_FrameBg, {0, 0, 0, 0});
        ImPlot::PushStyleColor(ImPlotCol_PlotBg, {0, 0, 0, 0});
        if (ImPlot::BeginPlot("fps", {vsScale * 5.0F, vsScale * 2.5F},
                              ImPlotFlags_NoInputs | ImPlotFlags_NoTitle | ImPlotFlags_NoLegend)) {
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
};