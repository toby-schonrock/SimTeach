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
#include "implot.h"

sf::Vector2f visualize(const Vec2& v) {
    return sf::Vector2f(static_cast<float>(v.x), static_cast<float>(v.y));
}

Vec2 unvisualize(const sf::Vector2f& v) { return Vec2(v.x, v.y); }

Vec2 unvisualize(const sf::Vector2i& v) { return Vec2(v.x, v.y); }

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
    // SFML
    sf::VideoMode       desktop = sf::VideoMode::getDesktopMode();
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Simulation", sf::Style::Fullscreen,
                            settings); //, sf::Style::Default);

    ImGui::SFML::Init(window);
    ImPlot::CreateContext();
    ImGuiIO& imguIO = ImGui::GetIO();
    imguIO.ConfigFlags &=
        ~ImGuiConfigFlags_NoMouseCursorChange; // omg all it took was this one ****ing line (disable
                                               // cursor overide)

    // nesecary sim stuff
    EntityManager entities;

    GUI gui(desktop, window, 0.05F);

    // Sim sim1(entities, 0.2F);                                                          // empty
    Sim sim1 = Sim::softbody(entities, {25, 25}, {14, 1}, 2.0F, 0.2F, 10000, 100); // default
    // Sim sim1 = Sim::softbody(entities, {100, 100}, {1, -10}, 2.0F, 0.1F, 100000, 100); // stress

    std::size_t                        selectedTool = 0;
    std::vector<std::unique_ptr<Tool>> tools;
    tools.push_back(std::make_unique<PointTool>(window, entities, "Points"));
    tools.push_back(std::make_unique<SpringTool>(window, entities, "Springs"));
    tools.push_back(std::make_unique<CustomPolyTool>(window, entities, "Custom Poly"));

    bool                                  running = false;
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
            while ((sinceVFrame.count() < 10'000'000)) { // TODO: min max avg frames test
                ++simFrames;
                std::chrono::system_clock::time_point frameTime =
                    std::chrono::high_resolution_clock::now();
                constexpr std::chrono::nanoseconds maxFrame{1'000'000}; // 1 milisecond
                std::chrono::nanoseconds           deltaTime = std::min(frameTime - last, maxFrame);
                last                                         = frameTime;

                sim1.simFrame(static_cast<double>(deltaTime.count()) / 1e9);
                sinceVFrame = frameTime - start;
            }
        } else {
            while ((sinceVFrame.count() < 10'000'000)) {
                sinceVFrame = std::chrono::high_resolution_clock::now() - start;
            } // spin untill frame has passed
        }

        sf::Vector2i mousePos = sf::Mouse::getPosition(
            window); // mouse position is only accurate to end of simulation frames (it does change)

        // poll events for sfml and imgui
        sf::Event event; // NOLINT
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyPressed &&
                       event.key.code == sf::Keyboard::Space) {
                if (!running) tools[selectedTool]->unequip();
                running = !running;
            } else if (!(imguIO.WantCaptureMouse && event.type == sf::Event::MouseButtonPressed)) {
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
            tools[selectedTool]->frame(sim1, mousePos);
        }

        gui.frame(entities, mousePos);

        ImGui::SFML::Render(window);
        window.display();

        sinceVFrame       = std::chrono::high_resolution_clock::now() - start;
        const double Vfps = 1e9 / static_cast<double>(sinceVFrame.count());
        const double Sfps = Vfps * simFrames;
        gui.fps.add({Vfps, Sfps});
    }

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();

    return 0;
}