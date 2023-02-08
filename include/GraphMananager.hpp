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
    RingBuffer<float> tvalues;
    bool              hasDumped   = false;
    std::size_t       graphBuffer;

    GraphManager(EntityManager& entities_, std::size_t graphBuffer_ = 5000) : entities(entities_), tvalues(graphBuffer_), graphBuffer(graphBuffer_) {}

    void updateDraw(float t) {
        ImGui::Begin("Graphs");
        tvalues.add(t);
        for (std::size_t i = 0; i != entities.graphs.size(); ++i) {
            entities.graphs[i].add(entities);
            entities.graphs[i].draw(i, tvalues);
        }
        ImGui::End();
    }

    void addGraph(const std::size_t& index, ObjectType type, Property prop, Component comp) {
        entities.graphs.emplace_back(index, type, prop, comp, graphBuffer);
    }

    void reset() {
        tvalues = RingBuffer<float>(graphBuffer);
        for (Graph& g: entities.graphs) {
            g.data = RingBuffer<float>(graphBuffer);
        }
        hasDumped = false;
    }

    void dumpData() {
        hasDumped = true;
        if (entities.graphs.empty()) throw std::runtime_error("Graphs are empty cannot dump data");

        // check for valid graphs
        std::vector<size_t> goodens;
        for (std::size_t i = 0; i != entities.graphs.size(); ++i) {
            if (!entities.graphs[i].data.v.empty()) {
                goodens.push_back(i); // add valid graphs to "goodens" list
            }
        }
        if (goodens.empty()) {
            std::cout << "No data to plot nothing saved \n";
            return;
        }

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
        p.make_preferred();
        std::cout << "Storing graph data at: " << p << "\n";
        std::ofstream file{p, std::ios_base::out};
        if (!file.is_open()) {
            throw std::logic_error("Falied to open fstream \n");
        }

        file << std::fixed << std::setprecision(10);
        // headers
        file << "Time";
        for (const std::size_t gooden: goodens) {
            file << "," << entities.graphs[gooden].getYLabel();
        }
        file << "\n";

        // data
        std::size_t i = tvalues.pos;
        do {
            file << tvalues.v[i];
            for (const std::size_t gooden: goodens) {
                file << "," << entities.graphs[gooden].data.v[i];
            }
            file << "\n";
            ++i;
            if (i == tvalues.size) i = 0; // handle wrap around
        } while (i != tvalues.pos);
    }
};