#include <chrono>

#include "imgui-SFML.h"
#include "imgui.h"
#include "implot.h"

#include "physenv/Engine.hpp"

#include "EntityManager.hpp"

int main() {
    // SFML
    sf::VideoMode       desktop = sf::VideoMode::getDesktopMode();
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(desktop, "Simulation", sf::Style::Default, settings);

    ImGui::SFML::Init(window);
    ImPlot::CreateContext();
    ImGuiIO& imguIO = ImGui::GetIO();
    imguIO.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange; // disable cursor overide

    // necessary sim stuff
    EntityManager entities{physenv::Engine::softbody({10, 10}, {0, 0}, 10, 1, 1, 1)};

    sf::Clock deltaClock; // for imgui https://eliasdaler.github.io/using-imgui-with-sfml-pt1/
    std::chrono::time_point epoch          = std::chrono::high_resolution_clock::now();
    std::chrono::time_point lastFrameStart = epoch;
    while (window.isOpen()) { // run for 1 seconds
        std::chrono::time_point  frameStart = std::chrono::high_resolution_clock::now();
        std::chrono::nanoseconds deltaTime  = frameStart - lastFrameStart;
        lastFrameStart                      = frameStart;

        entities.engine.simFrame(static_cast<double>(deltaTime.count()) / 1e9);
        // std::cout << "made it" << std::endl;

        sf::Event event; // NOLINT
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart()); // required for imgui-sfml
        window.clear();
        entities.drawPoints(window, 0.05f);
        entities.drawSprings(window);

        ImGui::SFML::Render(window);
        window.display();
    }

    ImPlot::DestroyContext();
    ImGui::SFML::Shutdown();

    return 0;
}