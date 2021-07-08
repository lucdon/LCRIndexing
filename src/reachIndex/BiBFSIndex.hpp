#pragma once

#include "ReachabilityIndex.hpp"

class BiBFSIndex : public ReachabilityIndex {
private:
    inline static const std::string indexName = "Bi-directional BFS";

public:
    void train() override { }
    bool query(const ReachQuery& query) override;

    [[nodiscard]] size_t indexSize() const override { return 0; }
    [[nodiscard]] const std::string &getName() const override { return indexName; }
};
