#pragma once

#include "WalkerLCRQueryGenerator.hpp"

class WalkerEmitRandomQueries {
private:
    std::random_device randomDevice;
    std::default_random_engine randomEngine;

public:
    WalkerEmitRandomQueries() : randomEngine(randomDevice()) { }

    void setup(WalkerLCRQueryGenerator &queryGenerator) {
        queryGenerator.setEmitQueryStrategy(
                std::bind(&WalkerEmitRandomQueries::onEmit, this, std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    }
private:
    bool onEmit(const LabeledEdgeGraph &labeledGraph, const Edge &vertex, const std::deque<Edge> &pathStack,
                LCRQuery &outQuery, bool isForward, uint32_t labelCount) {
        std::uniform_int_distribution<Vertex> distribution(0, labeledGraph.getVertexCount() - 1);

        outQuery.source = distribution(randomEngine);
        outQuery.target = distribution(randomEngine);

        outQuery.labelSet.resize(labeledGraph.getLabelCount());

        std::uniform_int_distribution<Label> labelDistribution(0, labeledGraph.getLabelCount() - 1);
        uint32_t selectedLabels = 0;

        while (selectedLabels < labelCount) {
            auto label = labelDistribution(randomEngine);

            if (!outQuery.labelSet[label]) {
                outQuery.labelSet[label] = true;
                selectedLabels++;
            }
        }

        return outQuery.source != outQuery.target;
    }
};