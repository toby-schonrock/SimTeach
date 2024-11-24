#include <chrono>

#include "physenv/Engine.hpp"

int main() {
    // necessary sim stuff
    physenv::Engine eng = physenv::Engine::softbody({10, 10}, {0, 0}, 10, 1, 1, 1);

    std::chrono::time_point launchTime     = std::chrono::high_resolution_clock::now();
    std::chrono::time_point lastFrameStart = launchTime;
    while ((lastFrameStart - launchTime).count() < 1'000'000'000) { // run for 1 seconds
        std::chrono::time_point  frameStart = std::chrono::high_resolution_clock::now();
        std::chrono::nanoseconds deltaTime  = frameStart - lastFrameStart;
        lastFrameStart                      = frameStart;

        eng.simFrame(static_cast<double>(deltaTime.count()) / 1e9);
    }
    return 0;
}