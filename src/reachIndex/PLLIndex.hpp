#pragma once

#include "ReachabilityIndex.hpp"

// Pruned landmark: https://arxiv.org/pdf/1304.4661.pdf
class PLLIndex : public ReachabilityIndex {
private:
    std::vector<std::vector<Vertex>> reachTo;
    std::vector<std::vector<Vertex>> reachFrom;

    inline static const std::string indexName = "Pruned Landmark Labeling";

public:
    explicit PLLIndex() {
        requiresComponentGraphDuringQueries = false;
    }

    void train() override;
    bool query(const ReachQuery &query) override;
    [[nodiscard]] size_t indexSize() const override;

    [[nodiscard]] const std::string &getName() const override { return indexName; }

private:
    void prunedBFS(boost::dynamic_bitset<> &visited, std::vector<Vertex> &queue, uint32_t label, Vertex landmark);
    void reversePrunedBFS(boost::dynamic_bitset<> &visited, std::vector<Vertex> &queue, uint32_t label, Vertex landmark);

    bool isReachable(Vertex source, Vertex target);
};
