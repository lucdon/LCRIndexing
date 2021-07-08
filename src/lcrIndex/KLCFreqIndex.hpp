#pragma once

#include <reachIndex/ReachabilityIndex.hpp>
#include <reachIndex/BFLIndex.hpp>
#include <utility>
#include "Index.hpp"

namespace lcr {
    class KLCFreqIndex : public Index {
    private:
        std::string indexName;

        uint32_t maxCombinations;

        uint32_t maxAboveCombinations = 0u;
        uint32_t maxPerVertexSetCount = 0u;
        uint32_t maxTotalSetCount = 0u;

        uint32_t minLabelsAboveCombinations = 0u;
        uint32_t maxLabelsAboveCombinations = 0u;

        std::string reachIndexName;
        uint32_t reachIndexOptionalParam;

        std::vector<std::unique_ptr<ReachabilityIndex>> singleLabelIndices;
        std::vector<std::pair<LabelSet, ReachabilityIndex *>> aboveLookup;
        std::unordered_map<LabelSet, std::unique_ptr<ReachabilityIndex>> indices;
        std::vector<std::unique_ptr<SCCGraph>> sccGraphs;

        std::unique_ptr<ReachabilityIndex> allIndex;
        std::unique_ptr<SCCGraph> allSccGraph;
    public:
        KLCFreqIndex(uint32_t k, std::string reachIndexName, uint32_t reachIndexOptionalParam) : maxCombinations(k),
                                                                                                 reachIndexName(
                                                                                                         std::move(
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
                indexName = "KLCF k=" + std::to_string(maxCombinations) + " (" + reachIndexName + ")";

                return;
            }

            uint32_t labelCount = labeledGraph->getLabelCount();

            if (labelCount <= 4) {
                // For very small label counts, just do everything
                maxCombinations = labelCount;
                indexName = "KLCF k=" + std::to_string(maxCombinations) + " (" + reachIndexName + ")";
                return;
            }

            maxCombinations = 2;

            minLabelsAboveCombinations = std::min<uint32_t>(std::max<uint32_t>(7u, labelCount / 4u),
                                                            std::max<uint32_t>(labelCount, 7u) - 4u);
            maxLabelsAboveCombinations = std::min<uint32_t>(std::max<uint32_t>(9u, labelCount / 2u),
                                                            std::max<uint32_t>(labelCount, 7u) - 2u);

            maxAboveCombinations = std::min<uint64_t>(1000ull, calculateCombinationsUpToK(maxCombinations, labelCount));

            indexName = "KLCF k=" + std::to_string(maxCombinations) + " k_min=" +
                        std::to_string(minLabelsAboveCombinations) + " k_max=" +
                        std::to_string(maxLabelsAboveCombinations) + " above=" + std::to_string(maxAboveCombinations) +
                        " (" + reachIndexName + ")";

            maxPerVertexSetCount = 100'000;
            maxTotalSetCount = 5'000'000;
        }

    private:
        void createSingleIndex(Label label);
        bool createIndex(const LabelSet &labelSet, bool isAbove);

        bool defaultStrategy(const LCRQuery &query, ReachabilityIndex *&bestBound);

        bool
        queryBelowCombinations(const ReachQuery &query, const LabelSet &labelSet, const std::vector<Label> &labels);
        int8_t queryAboveCombinations(const ReachQuery &query, const LabelSet &labelSet, ReachabilityIndex *&bestBound);

        void countFrequencies(std::stack<VertexLabelSet> &queue, Vertex vertex, VertexLabelSetVisitedSet &visited,
                              std::unordered_map<LabelSet, uint32_t> &frequencyMap, uint32_t &setCount);

        bool queryForCombination(const ReachQuery &reachQuery, LabelSet &labelSet, const std::vector<Label> &labels,
                                 uint32_t start, uint32_t end, uint32_t index);
    };
}