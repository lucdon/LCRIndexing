#pragma once

#include "ReachabilityIndex.hpp"


// Pruned landmark and paths: https://dl.acm.org/doi/10.1145/2505515.2505724
class PPLIndex : public ReachabilityIndex {
private:
    std::vector<std::vector<std::pair<Vertex, uint32_t>>> reachToPath;
    std::vector<std::vector<std::pair<Vertex, uint32_t>>> reachFromPath;

    std::vector<std::vector<Vertex>> reachTo;
    std::vector<std::vector<Vertex>> reachFrom;

    inline static const std::string indexName = "Pruned Path Labeling";

public:
    explicit PPLIndex() {
        requiresComponentGraphDuringQueries = false;
    }

    void train() override;
    bool query(const ReachQuery &query) override;
    [[nodiscard]] size_t indexSize() const override;

    [[nodiscard]] const std::string &getName() const override { return indexName; }

private:
    bool isReachable(Vertex source, Vertex target);

    void buildOptimalPath(boost::dynamic_bitset<>&  used, std::vector<Vertex>& path);

    void prunedBFS(boost::dynamic_bitset<> &visited, boost::dynamic_bitset<> &used, std::queue<Vertex> &queue,
                   std::vector<Vertex>& visitedVertex, std::vector<Vertex>& path, uint32_t pathLabel);

    void reversePrunedBFS(boost::dynamic_bitset<> &visited, boost::dynamic_bitset<> &used, std::queue<Vertex> &queue,
                          std::vector<Vertex>& visitedVertex, std::vector<Vertex>& path, uint32_t pathLabel);
};
