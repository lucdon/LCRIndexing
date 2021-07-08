#pragma once

#include "WalkerQueryGenerator.hpp"

class WalkerStartOnBestDegree {
private:
    std::vector<Vertex> topVertices { };

    double percentage = -1;
    uint32_t flatAmount = 0;

    std::random_device randomDevice;
    std::default_random_engine randomEngine;

public:
    WalkerStartOnBestDegree() : randomEngine(randomDevice()) { }

    explicit WalkerStartOnBestDegree(uint32_t flatAmountToInclude) : WalkerStartOnBestDegree() {
        flatAmount = flatAmountToInclude;
    }

    void setPercentage(double percentageToInclude) {
        if (percentageToInclude > 1.0 || percentageToInclude < 0.0) {
            std::cerr << "percentage out of [0.0, 1.0] bound: " << percentageToInclude << std::fatal;
        }

        percentage = percentageToInclude;
    }

    void setup(WalkerQueryGenerator &queryGenerator) {
        queryGenerator.subscribeOnReset(
                std::bind(&WalkerStartOnBestDegree::onReset, this, std::placeholders::_1, std::placeholders::_2));

        queryGenerator
                .setPlacementStrategy(std::bind(&WalkerStartOnBestDegree::onPlace, this, std::placeholders::_1));
    }
private:
    void onReset(const SCCGraph &sccGraph, bool firstTime) {
        auto &graph = sccGraph.getComponentGraph();

        if (firstTime) {
            if (percentage != -1) {
                flatAmount = percentage * ((double) graph.getVertexCount());
            }

            std::vector<std::pair<int, Vertex>> degreeByVertex(graph.getVertexCount());

            for (int vertex = 0u; vertex < graph.getVertexCount(); vertex++) {
                uint32_t outgoingDegree = graph.getConnected(vertex).size();
                uint32_t incomingDegree = graph.getReverseConnected(vertex).size();

                int degree = outgoingDegree - incomingDegree;

                degreeByVertex[vertex] = std::make_pair(degree, vertex);
            }

            std::sort(degreeByVertex.begin(), degreeByVertex.end(), std::greater<>());

            topVertices.resize(std::min(flatAmount, (uint32_t) degreeByVertex.size()));

            for (uint32_t i = 0; i < flatAmount && i < degreeByVertex.size(); i++) {
                topVertices[i] = degreeByVertex[i].second;
            }
        }
    }

    Vertex onPlace(const SCCGraph &sccGraph) {
        std::uniform_int_distribution<Vertex> distribution(0, topVertices.size() - 1);
        return topVertices[distribution(randomEngine)];
    }
};