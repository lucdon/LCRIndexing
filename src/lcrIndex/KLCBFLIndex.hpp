#pragma once

#include <reachIndex/ReachabilityIndex.hpp>
#include <utility>
#include "Index.hpp"

namespace lcr {
    class KLCBFLIndex : public Index {
    private:
        std::string indexName;
        uint32_t maxCombinations;

        std::vector<std::unique_ptr<ReachabilityIndex>> singleLabelIndices;
        std::vector<std::pair<LabelSet, std::unique_ptr<ReachabilityIndex>>> indices;
        std::vector<std::unique_ptr<SCCGraph>> sccGraphs;

        std::unique_ptr<ReachabilityIndex> allIndex;
        std::unique_ptr<SCCGraph> allSccGraph;
    public:
        KLCBFLIndex(uint32_t k) : maxCombinations(k) { }

        void train() override;
        bool query(const LCRQuery &query) override;
        QueryResult queryOnce(const LCRQuery& query) override;

        [[nodiscard]] size_t indexSize() const override;
        [[nodiscard]] const std::string &getName() const override { return indexName; }

        void setGraph(LabeledEdgeGraph *labeledGraph) override {
            Index::setGraph(labeledGraph);

            if (maxCombinations != std::numeric_limits<uint32_t>::max()) {
                indexName = "KLC-BFL k=" + std::to_string(maxCombinations);

                return;
            }

            uint32_t labelCount = labeledGraph->getLabelCount();

            if (labelCount <= 4) {
                // For very small label counts, just do everything
                maxCombinations = labelCount;
                indexName = "KLC-BFL k=" + std::to_string(maxCombinations);
                return;
            }

            maxCombinations = 2;
            indexName = "KLC-BFL k=" + std::to_string(maxCombinations);
        }

    private:
        void createSingleIndex(Label label);
        bool createIndex(const LabelSet &labelSet);

        bool defaultStrategy(const LCRQuery &query, std::vector<ReachabilityIndex *> &reachIndexes);

        void gatherReachIndexes(const ReachQuery &query, const LabelSet &labelSet,
                                std::vector<ReachabilityIndex *> &reachIndexes);
    };
}