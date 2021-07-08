#pragma once

#include "reachIndex/ReachabilityIndex.hpp"
#include "utility/Limit.hpp"

class ReachQueriesRunner {

private:
    std::unique_ptr<ReachabilityIndex> controlIndex = nullptr;
    std::vector<std::unique_ptr<ReachabilityIndex>> indices;

    std::unique_ptr<Limit> limit = nullptr;
public:
    void setControlIndex(std::unique_ptr<ReachabilityIndex> &&index) {
        controlIndex = std::move(index);
    }

    void addIndex(std::unique_ptr<ReachabilityIndex> &&index) {
        indices.emplace_back(std::move(index));
    }

    void setLimit(std::unique_ptr<Limit> &&lim) {
        limit = std::move(lim);
    }

    void run(std::string &graphFile, const std::vector<std::string> &queryFile);
};
