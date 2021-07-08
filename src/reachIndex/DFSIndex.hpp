#pragma once

#include "ReachabilityIndex.hpp"

class DFSIndex : public ReachabilityIndex {
private:
    inline static const std::string indexName = "DFS";

public:
    void train() override { }
    bool query(const ReachQuery& query) override;

    [[nodiscard]] size_t indexSize() const override { return 0; }
    [[nodiscard]] const std::string &getName() const override { return indexName; }
};
