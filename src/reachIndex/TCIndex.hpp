#pragma once

#include "ReachabilityIndex.hpp"

class TCIndex : public ReachabilityIndex {
private:
    inline static const std::string indexName = "Full Transitive Closure";

    std::vector<std::vector<bool>> closure;

public:
    TCIndex() {
        requiresComponentGraphDuringQueries = false;
    }

    void train() override;
    bool query(const ReachQuery& query) override;

    [[nodiscard]] size_t indexSize() const override;
    [[nodiscard]] const std::string &getName() const override { return indexName; }
};
