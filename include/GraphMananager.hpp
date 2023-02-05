#pragma once

#include "EntityManager.hpp"
#include "Graph.hpp"
#include <cstddef>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>

class GraphManager {
  private:
    EntityManager& entities;

  public:
    bool        hasDumped   = false;
    std::size_t graphBuffer = 5000;
    GraphManager(EntityManager& entities_) : entities(entities_) {}

    void updateDraw(float t) {
        ImGui::Begin("Graphs");
        for (std::size_t i = 0; i != entities.graphs.size(); ++i) {
            entities.graphs[i].add(t, entities);
            entities.graphs[i].draw(i);
        }
        ImGui::End();
    }

    void addGraph(const std::size_t& index, ObjectType type, Property prop, Component comp) {
        entities.graphs.emplace_back(index, type, prop, comp, graphBuffer);
    }

    void reset() {
        for (Graph& g: entities.graphs) {
            g.data = RingBuffer<Vec2F>(graphBuffer);
        }
        hasDumped = false;
    }

    void dumpData() {
        // get time and date
        const std::chrono::time_point     now{std::chrono::system_clock::now()};
        const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};
        const std::chrono::hh_mm_ss       hms{now - std::chrono::floor<std::chrono::days>(now)};
        std::string name = std::to_string(static_cast<int>(ymd.year())) + "-" +
                           std::to_string(static_cast<unsigned>(ymd.month())) + "-" +
                           std::to_string(static_cast<unsigned>(ymd.day())) + "_" +
                           std::to_string(static_cast<unsigned>(hms.hours().count())) + "." +
                           std::to_string(static_cast<unsigned>(hms.minutes().count())) + "." +
                           std::to_string(static_cast<unsigned>(hms.seconds().count()));
        name.pop_back();
        std::replace(name.begin(), name.end(), ' ', '-');
        std::filesystem::create_directory("graphdata");
        std::filesystem::path p = "graphdata/" + name + ".csv";
        std::cout << "Storing graph data at: " << p.make_preferred() << "\n";
        std::ofstream file{p, std::ios_base::out};
        if (!file.is_open()) {
            throw std::logic_error("Falied to open fstream \n");
        }
        for (std::size_t i = 0; i != entities.graphs.size(); ++i) {
            const Graph& g = entities.graphs[i];
            file << "Time," << g.getYLabel() << "\n";
            for (const Vec2F& d: g.data.v) {
                file << d.x << "," << d.y << "\n";
            }
        }
        hasDumped = true;
    }
};