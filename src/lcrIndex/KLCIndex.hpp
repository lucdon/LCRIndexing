#pragma once

#include <reachIndex/ReachabilityIndex.hpp>
#include <utility>
#include "Index.hpp"

namespace lcr {
    class KLCIndex : public Index {
    private:
        std::string indexName;

        uint32_t maxCombinations;

        std::string reachIndexName;
        uint32_t reachIndexOptionalParam;

        std::vector<std::unique_ptr<ReachabilityIndex>> singleLabelIndices;
        std::unordered_map<LabelSet, std::unique_ptr<ReachabilityIndex>> indices;
        std::vector<std::unique_ptr<SCCGraph>> sccGraphs;

        std::unique_ptr<ReachabilityIndex> allIndex;
        std::unique_ptr<SCCGraph> allSccGraph;
    public:
        KLCIndex(uint32_t k, std::string reachIndexName, uint32_t reachIndexOptionalParam) : maxCombinations(k),
                                                                                             reachIndexName(std::move(
                                                                                                     reachIndexName)),
                                                                                             reachIndexOptionalParam(
                                                                                                     reachIndexOptionalParam) {
        }

        void train() override;
        bool query(const LCRQuery &query) override;
        QueryResult queryOnce(const LCRQuery& query) override;

        [[nodiscard]] size_t indexSize() const override;
        [[nodiscard]] const std::string &getName() const override { return indexName; }

        void setGraph(LabeledEdgeGraph *labeledGraph) override {
            Index::setGraph(labeledGraph);

            if (maxCombinations != std::numeric_limits<uint32_t>::max()) {
                indexName = "KLC k=" + std::to_string(maxCombinations) + " (" + reachIndexName + ")";

                return;
            }

            uint32_t labelCount = labeledGraph->getLabelCount();

            if (labelCount <= 4) {
                // For very small label counts, just do everything
                maxCombinations = labelCount;
                indexName = "KLC k=" + std::to_string(maxCombinations) + " (" + reachIndexName + ")";
                return;
            }

            maxCombinations = 2;
            indexName = "KLC k=" + std::to_string(maxCombinations) + " (" + reachIndexName + ")";
        }

    private:
        void createSingleIndex(Label label);
        bool createIndex(const LabelSet &labelSet);

        bool defaultStrategy(const LCRQuery &query);

        bool queryBelowCombinations(const ReachQuery &query, const LabelSet &labelSet, const std::vector<Label>& labels);
        bool queryForCombination(const ReachQuery &reachQuery, LabelSet &labelSet, const std::vector<Label> &labels,
                                 uint32_t start, uint32_t end, uint32_t index);
    };
}