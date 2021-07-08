#pragma once

#include "WalkerQueryGenerator.hpp"

class WalkerEmitTrueAndFalseQueries {
private:
    std::random_device randomDevice;
    std::default_random_engine randomEngine;

    double trueFalseDistribution = 0.5;

public:
    WalkerEmitTrueAndFalseQueries() : randomEngine(randomDevice()) { }

    explicit WalkerEmitTrueAndFalseQueries(double trueFalseDist) : WalkerEmitTrueAndFalseQueries() {
        if (trueFalseDist > 1.0 || trueFalseDist < 0.0) {
            std::cerr << "percentage out of [0.0, 1.0] bound: " << trueFalseDist << std::fatal;
        }

        trueFalseDistribution = trueFalseDist;
    }

    void setup(WalkerQueryGenerator &queryGenerator) {
        queryGenerator.setEmitQueryStrategy(
                std::bind(&WalkerEmitTrueAndFalseQueries::onEmit, this, std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    }
private:
    bool onEmit(const SCCGraph &sccGraph, Vertex vertex, const std::deque<Vertex> &pathStack, ReachQuery &outQuery,
                bool isForward) {
        std::bernoulli_distribution emitDistribution(0.5);

        if (!emitDistribution(randomEngine)) {
            return false;
        }

        std::uniform_int_distribution<Vertex> distribution(0, pathStack.size() - 1);

        if (isForward) {
            outQuery.source = pathStack[distribution(randomEngine)];
            outQuery.target = vertex;
        } else {
            outQuery.source = vertex;
            outQuery.target = pathStack[distribution(randomEngine)];
        }

        if (outQuery.source == outQuery.target) {
            return false;
        }

        std::bernoulli_distribution emitTrueFalseDistribution(trueFalseDistribution);

        if (!emitTrueFalseDistribution(randomEngine)) {
            std::uniform_int_distribution<Vertex> distribution2(0, sccGraph.getComponentGraph().getVertexCount() - 1);

            if (isForward) {
                outQuery.source = distribution2(randomEngine);
                outQuery.target = vertex;
            } else {
                outQuery.source = vertex;
                outQuery.target = distribution2(randomEngine);
            }
        }

        return outQuery.source != outQuery.target;
    }
};