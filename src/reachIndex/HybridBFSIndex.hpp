#pragma once

#include "ReachabilityIndex.hpp"

class HybridBFSIndex : public ReachabilityIndex {
private:
    inline static const std::string indexName = "Hybrid BFS a=14 b=24";

public:
    void train() override;
    bool query(const ReachQuery &query) override;

    [[nodiscard]] size_t indexSize() const override { return 0; }

    [[nodiscard]] const std::string &getName() const override {
        return indexName;
    }

private:
    static bool bottomUp(const DiGraph &graph, Vertex target, uint32_t &nodesInFrontier, uint32_t &edgesInFrontier);
    static bool topDown(const DiGraph &graph, Vertex target, uint32_t &nodesInFrontier, uint32_t &edgesInFrontier);
};
