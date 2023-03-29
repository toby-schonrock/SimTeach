#include <array>
#include <chrono>
#include <iostream>
#include <vector>

#include "EntityManager.hpp"
#include "GUI.hpp"
#include "RingBuffer.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"
#include "Sim.hpp"
#include "Tool.hpp"
#include "Vector2.hpp"
#include "imgui-SFML.h"
#include "imgui.h"

const std::filesystem::path Previous{"previous.csv"};

sf::Vector2f visualize(const Vec2& v) {
    return sf::Vector2f(static_cast<float>(v.x), -static_cast<float>(v.y));
}

Vec2 unvisualize(const sf::Vector2f& v) { return Vec2(v.x, -v.y); }

Vec2 unvisualize(const sf::Vector2i& v) { return Vec2(v.x, -v.y); }

int main() {
    // SFML
    sf::VideoMode       desktop = sf::VideoMode::getDesktopMode();
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Simulation", sf::Style::Fullscreen,
                            settings);

    ImGui::SFML::Init(window);
    ImPlot::CreateContext();
    ImGuiIO& imguIO = ImGui::GetIO();
    imguIO.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange; // disable cursor overide

    // necessary sim stuff
    EntityManager entities;

    GUI          gui(entities, desktop, window, 0.05F);
    GraphManager graphs{entities};

    Sim sim(entities, 2.0F);

    std::size_t                        selectedTool = 0;
    std::vector<std::unique_ptr<Tool>> tools;
    tools.push_back(std::make_unique<PointTool>(window, entities, "Points"));
    tools.push_back(std::make_unique<SpringTool>(window, entities, "Springs"));
    tools.push_back(std::make_unique<PolyTool>(window, entities, "Polys"));
    tools.push_back(std::make_unique<GraphTool>(window, entities, graphs, "Graphs"));

    bool                                  running = false;
    std::chrono::system_clock::time_point runtime;
    std::chrono::system_clock::time_point last =
        std::chrono::high_resolution_clock::now(); // setting time of previous frame to be now
    sf::Clock
        deltaClock; // for imgui - read https://eliasdaler.github.io/using-imgui-with-sfml-pt1/
    while (window.isOpen()) {
        std::chrono::system_clock::time_point start = std::chrono::high_resolution_clock::now();

        // run the sim
        int                      simFrames   = 0;
        std::chrono::nanoseconds sinceVFrame = std::chrono::high_resolution_clock::now() - start;
        if (running) {
            while (sinceVFrame.count() < 10'000'000) {
                ++simFrames;
                std::chrono::system_clock::time_point frameTime =
                    std::chrono::high_resolution_clock::now();
                constexpr std::chrono::nanoseconds maxFrame{1'000'000}; // 1 milisecond
                std::chrono::nanoseconds           deltaTime = std::min(frameTime - last, maxFrame);
                last                                         = frameTime;

                sim.simFrame(static_cast<double>(deltaTime.count()) / 1e9);
                sinceVFrame = frameTime - start;
            }
        } // not running spin moved to end

        sf::Vector2i mousePos = sf::Mouse::getPosition(
            window); // mouse position is only accurate to end of simulation frames (it does change)

        // poll events for sfml and imgui
        sf::Event event; // NOLINT
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyPressed &&
                       event.key.code == sf::Keyboard::Space && !imguIO.WantCaptureKeyboard) {
                if (running) { // when space bar to stop
                    running = false;
                } else { // when space bar to run
                    sim.save(Previous, {true, true, true});
                    tools[selectedTool]->unequip();
                    graphs.reset();
                    runtime = std::chrono::high_resolution_clock::now();
                    running = true;
                }
            } else if (!running && event.type == sf::Event::KeyPressed &&
                       event.key.code == sf::Keyboard::R && !imguIO.WantCaptureKeyboard) {
                sim.reset();
            } else {
                gui.event(event, mousePos);
                if (!running) tools[selectedTool]->event(event);
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart()); // required for imgui-sfml

        // draw
        window.clear();

        if (!running) {
            ImGui::Begin("Tool Settings", NULL,
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::SetWindowSize({-1.0F, -1.0F}, ImGuiCond_Always);
            sf::Vector2u windowSize = window.getSize();
            ImVec2       IMSize     = ImGui::GetWindowSize();
            ImGui::SetWindowPos({static_cast<float>(windowSize.x) - IMSize.x, -1.0F},
                                ImGuiCond_Always);
            if (ImGui::BeginTabBar("Tools")) {
                for (std::size_t i = 0; i < tools.size(); ++i) {
                    if (ImGui::BeginTabItem(tools[i]->name.data())) {
                        if (selectedTool != i) tools[selectedTool]->unequip(); // if changed
                        selectedTool = i;
                        ImGui::EndTabItem();
                    }
                }
                ImGui::EndTabBar();
            }
            tools[selectedTool]->ImTool();
            ImGui::End();
            tools[selectedTool]->frame(sim, mousePos);
        } else {
            graphs.updateDraw(
                static_cast<float>((std::chrono::high_resolution_clock::now() - runtime).count()) /
                1e9F);
        }

        gui.frame(mousePos, sim, graphs, running);

        ImGui::SFML::Render(window);
        window.display();

        while ((std::chrono::high_resolution_clock::now() - start).count() <
               10'001'000) { // spin to make 100 VFps
        }
        sinceVFrame       = std::chrono::high_resolution_clock::now() - start;
        const double Vfps = 1e9 / static_cast<double>(sinceVFrame.count());
        const double Sfps = Vfps * simFrames;
        gui.fps.add({Vfps, Sfps});
    }

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();

    return 0;
}