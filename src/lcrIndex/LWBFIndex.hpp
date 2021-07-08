#pragma once

#include "Index.hpp"

namespace lcr {
    struct LWBFTrainState {
    public:
        VertexQueueVec queue;

        boost::dynamic_bitset<> landmarkVisited;
        std::vector<std::map<LabelSet, BloomFilter1Hash>> incomingLabels;

        std::vector<std::vector<LabelSet>> vertexLookup;

        LWBFTrainState(uint32_t numBloomFilters, uint32_t vertexCount) {
            vertexLookup.resize(vertexCount);
            landmarkVisited.resize(vertexCount);
            incomingLabels.resize(numBloomFilters);
        }
    };

    class LWBFIndex : public Index {
    private:
        std::string indexName;

        uint32_t landmarkCount;
        uint32_t numBloomFilters;
        uint32_t bloomFilterBits;

        std::vector<std::vector<std::pair<LabelSet, BloomFilter1Hash>>> outgoingLabels;
        std::vector<std::vector<std::pair<LabelSet, BloomFilter1Hash>>> incomingLabels;

        std::vector<uint32_t> landmarkMapping;
        std::vector<uint32_t> bloomFilterMapping;

        std::vector<std::vector<std::pair<Vertex, LabelSet>>> landmarkMap;

    public:
        explicit LWBFIndex(uint32_t landmarkCount, uint32_t numBloomFilters,
                           uint32_t bloomFilterBits) : landmarkCount(landmarkCount), numBloomFilters(numBloomFilters),
                                                       bloomFilterBits(bloomFilterBits) {
            indexName = "Landmarks with bloom filters";
        }

        void setGraph(LabeledEdgeGraph *labeledGraph) override {
            Index::setGraph(labeledGraph);

            if (landmarkCount == std::numeric_limits<uint32_t>::max()) {
                landmarkCount = 2u * uint32_t(std::sqrt(double(labeledGraph->getVertexCount())));
            }

            landmarkCount = std::min<uint32_t>(labeledGraph->getVertexCount(), landmarkCount);

            if (numBloomFilters == std::numeric_limits<uint32_t>::max()) {
                numBloomFilters = labeledGraph->getVertexCount() / 4;
            }

            numBloomFilters = std::min<uint32_t>(labeledGraph->getVertexCount(), numBloomFilters);

            if (bloomFilterBits == std::numeric_limits<uint32_t>::max()) {
                bloomFilterBits = 5 * 32;
            }
        }

        void train() override;
        bool query(const LCRQuery &query) override;;

        QueryResult queryOnce(const LCRQuery& query) override;
        QueryResult queryOnceRecursive(const LCRQuery& query) override;

        [[nodiscard]] size_t indexSize() const override;

        [[nodiscard]] const std::string &getName() const override {
            return indexName;
        }

    private:
        bool isReachable(Vertex source, Vertex target, const LabelSet &labelSet);

        void forwardBFS(Vertex vertex, LWBFTrainState &trainState);
        void createBloomFilter(Vertex vertex, LWBFTrainState &trainState);
        void convertLandmarkToBloomFilter(Vertex vertex, LWBFTrainState &trainState);

        [[nodiscard]] bool isLandmark(Vertex vertex) const;
        [[nodiscard]] bool isBloomFilter(Vertex vertex) const;

        bool queryLandmark(Vertex landmark, Vertex target, const LabelSet &labelSet);
        bool queryLandmark(Vertex landmark, Vertex target, const LabelSet &labelSet, boost::dynamic_bitset<> &visited);

        void
        tryInsertLandmark(Vertex landmark, Vertex otherLandmark, const LabelSet &labelSet, LWBFTrainState &trainState);
        static bool tryInsert(Vertex landmark, Vertex target, const LabelSet &labelSet, LWBFTrainState &trainState);
    };
}