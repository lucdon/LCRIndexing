#pragma once

#include <reachIndex/ReachabilityIndex.hpp>
#include "Index.hpp"

namespace lcr {
    class ALCIndex : public Index {
    private:
        std::string indexName;

        std::string reachIndexName;
        uint32_t reachIndexOptionalParam;

        std::unordered_map<LabelSet, std::unique_ptr<ReachabilityIndex>> indices;
        std::unordered_map<LabelSet, std::unique_ptr<SCCGraph>> sccGraphs;
    public:
        ALCIndex(std::string reachIndexName, uint32_t reachIndexOptionalParam) : reachIndexName(reachIndexName),
                                                                                 reachIndexOptionalParam(
                                                                                   reachIndexOptionalParam) {
            indexName = "ALC (" +
                        ReachabilityIndex::create(reachIndexName, std::to_string(reachIndexOptionalParam))->getName() + ")";
        }

        void train() override;
        bool query(const LCRQuery &query) override;

        void createIndex(const LabelSet &labelSet);

        [[nodiscard]] size_t indexSize() const override;
        [[nodiscard]] const std::string &getName() const override { return indexName; }
    };
}
