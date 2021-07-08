#pragma once

#include "ReachabilityIndex.hpp"

class BFLIndex : public ReachabilityIndex {
private:
    std::string indexName;

    uint32_t labelSize;
    uint32_t labelBitSize;
    uint32_t intervalCount;

    std::vector<std::vector<uint32_t>> incomingLabels;                   // L_In
    std::vector<std::vector<uint32_t>> outgoingLabels;                   // L_Out

    std::vector<std::pair<uint32_t, uint32_t>> intervalLabels;              // L_int = [L_dis, L_fin]

    // Random state for the hash function.
    std::random_device dev;
    std::default_random_engine randomEngine;

    // State for the intervals.
    uint32_t currentHash;
    uint32_t intervalCounter;
    uint32_t maxCounter;

    uint32_t curVisited;
    std::vector<uint32_t> visited;

public:
    explicit BFLIndex(uint32_t k) : randomEngine(dev()) {
        labelSize = k;
        labelBitSize = k * 32;
        intervalCount = k * 32 * 10;

        indexName = "Bloom Filter Labeling k=" + std::to_string(k);

        currentHash = randomEngine();
        intervalCounter = 0u;
    }

    void train() override;
    bool query(const ReachQuery &query) override;
    [[nodiscard]] size_t indexSize() const override;

    bool queryOnce(const ReachQuery &query) override;

    [[nodiscard]] const std::string &getName() const override {
        return indexName;
    }

private:
    bool isReachable(const DiGraph &componentGraph, Vertex source, Vertex target);
    bool isReachableOnce(const DiGraph &componentGraph, Vertex source, Vertex target);

    void reverseDFS(Vertex target);
    void forwardDFS(Vertex source, uint32_t &intervalMarker);

    uint32_t hashGet();
    void hashReset();
};
