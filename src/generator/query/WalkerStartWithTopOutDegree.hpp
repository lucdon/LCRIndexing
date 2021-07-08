#pragma once

#include "WalkerQueryGenerator.hpp"

class WalkerStartWithTopOutDegree {
private:
    std::vector<Vertex> topVertices { };

    double percentage = -1;
    uint32_t flatAmount = 0;

    std::random_device randomDevice;
    std::default_random_engine randomEngine;

public:
    WalkerStartWithTopOutDegree() : randomEngine(randomDevice()) { }

    explicit WalkerStartWithTopOutDegree(uint32_t flatAmountToInclude) : WalkerStartWithTopOutDegree() {
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
                std::bind(&WalkerStartWithTopOutDegree::onReset, this, std::placeholders::_1, std::placeholders::_2));

        queryGenerator
                .setPlacementStrategy(std::bind(&WalkerStartWithTopOutDegree::onPlace, this, std::placeholders::_1));
    }
private:
    void onReset(const SCCGraph &sccGraph, bool firstTime) {
        auto &graph = sccGraph.getComponentGraph();

        if (firstTime) {
            if (percentage != -1) {
                flatAmount = percentage * ((double) graph.getVertexCount());
            }

            std::vector<std::pair<size_t, Vertex>> outgoingDegreeByVertex(graph.getVertexCount());

            for (int vertex = 0u; vertex < graph.getVertexCount(); vertex++) {
                uint32_t outgoingDegree = graph.getConnected(vertex).size();
                outgoingDegreeByVertex[vertex] = std::make_pair(outgoingDegree, vertex);
            }

            std::sort(outgoingDegreeByVertex.begin(), outgoingDegreeByVertex.end(), std::greater<>());

            topVertices.resize(std::min(flatAmount, (uint32_t) outgoingDegreeByVertex.size()));

            for (uint32_t i = 0; i < flatAmount && i < outgoingDegreeByVertex.size(); i++) {
                topVertices[i] = outgoingDegreeByVertex[i].second;
            }
        }
    }

    Vertex onPlace(const SCCGraph &sccGraph) {
        std::uniform_int_distribution<Vertex> distribution(0, topVertices.size() - 1);
        return topVertices[distribution(randomEngine)];
    }
};