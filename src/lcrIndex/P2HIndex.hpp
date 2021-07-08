#pragma once

#include "Index.hpp"

namespace lcr {
    typedef std::vector<std::vector<std::pair<Vertex, LabelSet>>> TwoHopIndex;
    typedef std::set<std::pair<Vertex, LabelSet>, FrontierSetComparatorWithOrder> FrontierSet;

    class P2HIndex : public Index {
    private:
        std::vector<Label> primaryLabelSet;

        TwoHopIndex primaryReachOut;
        TwoHopIndex primaryReachIn;

        TwoHopIndex secondaryReachOut;
        TwoHopIndex secondaryReachIn;

        std::vector<Label> virtualLabelMapping;
        uint32_t numMostFrequentLabels;

        std::string indexName = "Pruned 2-Hop w=";

    public:
        explicit P2HIndex() : numMostFrequentLabels(12) {
            indexName += std::to_string(12);
        }

        explicit P2HIndex(uint32_t numMostFrequentLabels) : numMostFrequentLabels(numMostFrequentLabels) {
            indexName += std::to_string(numMostFrequentLabels);
        }

        void train() override;
        bool query(const LCRQuery &query) override;
        QueryResult queryOnce(const LCRQuery& query) override;

        [[nodiscard]] size_t indexSize() const override;
        [[nodiscard]] const std::string &getName() const override { return indexName; }

    private:
        void buildPrimaryIndex(const LabeledEdgeGraph &graph, boost::dynamic_bitset<> &visited);
        void buildSecondaryIndex(const LabeledEdgeGraph &graph, boost::dynamic_bitset<> &visited);

        static void
        prunedBFS(const LabeledEdgeGraph &graph, TwoHopIndex &reachIn, TwoHopIndex &reachOut, FrontierSet &current,
                  FrontierSet &plusOne, FrontierSet &temp, boost::dynamic_bitset<> &visited, Vertex vertex);
        static void reversePrunedBFS(const LabeledEdgeGraph &graph, TwoHopIndex &reachIn, TwoHopIndex &reachOut,
                                     FrontierSet &current, FrontierSet &plusOne, FrontierSet &temp,
                                     boost::dynamic_bitset<> &visited, Vertex vertex);

        static bool insertToIndex(TwoHopIndex &index, Vertex source, Vertex target, const LabelSet &labelSet);

        bool isPrimaryReachable(Vertex source, Vertex target, const LabelSet &labels);
        bool isSecondaryReachable(Vertex source, Vertex target, const std::vector<Label> &labels);
        static bool isReachable(Vertex source, Vertex target, const LabelSet &labels, const TwoHopIndex &reachIn,
                                const TwoHopIndex &reachOut);

        bool defaultStrategy(const LCRQuery &query);
    };
}