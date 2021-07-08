#pragma once

#include "Index.hpp"

namespace lcr {
    class LandmarkPlusIndex : public Index {
    private:
        std::string indexName;

        uint32_t landmarkCount = 0;
        uint32_t nonLandmarkCount = 0;
        uint32_t minReachLength = 0;
        uint32_t maxReachableLabels = 0;

        std::vector<int64_t> landmarkMapping;
        std::vector<std::vector<std::pair<Vertex, LabelSet>>> landmarkMap;
        std::vector<std::vector<std::pair<Vertex, LabelSet>>> nonLandmarkMap;
        std::vector<std::vector<ReachableEntry>> reachableBy;

    public:
        explicit LandmarkPlusIndex(uint32_t landmarkCount, uint32_t nonLandmarkCount) : landmarkCount(landmarkCount),
                                                                                                       nonLandmarkCount(
                                                                                                               nonLandmarkCount) {
            indexName = "Landmark-plus k=" + std::to_string(landmarkCount) + " b=" + std::to_string(nonLandmarkCount);
        }

        void setGraph(LabeledEdgeGraph *labeledGraph) override {
            Index::setGraph(labeledGraph);
            maxReachableLabels = uint32_t(double(labeledGraph->getLabelCount()) / 4.0 + 1.0);
            minReachLength = (uint32_t) std::min(50.0 + std::sqrt(double(getGraph().getVertexCount())) / 2.0,
                                                 double(getGraph().getVertexCount()));

            if (landmarkCount == std::numeric_limits<uint32_t>::max()) {
                landmarkCount = 8u * uint32_t(std::sqrt(double(getGraph().getVertexCount())));
            }

            if (nonLandmarkCount == std::numeric_limits<uint32_t>::max()) {
                nonLandmarkCount = 20;
            }

            if (landmarkCount > labeledGraph->getVertexCount()) {
                landmarkCount = labeledGraph->getVertexCount();
                indexName = "Landmark-plus k=all b=0";
                return;
            }

            indexName = "Landmark-plus k=" + std::to_string(landmarkCount) + " b=" + std::to_string(nonLandmarkCount);
        }

        void train() override;
        bool query(const LCRQuery &query) override;
        QueryResult queryOnce(const LCRQuery& query) override;
        QueryResult queryOnceRecursive(const LCRQuery& query) override;

        [[nodiscard]] size_t indexSize() const override;
        [[nodiscard]] const std::string &getName() const override {
            return indexName;
        }

    private:
        void createIndexForLandmark(VertexReachQueue &queue, Vertex vertex,
                                    std::vector<std::vector<LabelSet>> &vertexLookup);
        void createIndexForNonLandmark(VertexReachQueue &queue, Vertex vertex,
                                       std::vector<std::vector<LabelSet>> &vertexLookup);

        bool queryExtensive(Vertex landmark, Vertex target, const LabelSet &labelSet, boost::dynamic_bitset<> &visited);
        bool queryLandmark(Vertex landmark, Vertex target, const LabelSet &labelSet);
        bool queryNonLandmark(Vertex vertex, Vertex target, const LabelSet &labelSet, boost::dynamic_bitset<> &visited);
        bool queryNonLandmark(Vertex vertex, Vertex target, const LabelSet &labelSet);

        [[nodiscard]] bool isLandmark(Vertex current) const;
        [[nodiscard]] bool isNonLandmark(Vertex current) const;

        [[nodiscard]] std::vector<std::pair<Vertex, LabelSet>> &getLandmark(Vertex current);
        [[nodiscard]] std::vector<std::pair<Vertex, LabelSet>> &getNonLandmark(Vertex current);
        [[nodiscard]] std::vector<ReachableEntry> &getReachableBy(Vertex current);

        void tryInsertLandmark(Vertex landmark, Vertex otherLandmark, const LabelSet &labelSet,
                               std::vector<std::vector<LabelSet>> &vertexLookup, uint32_t &totalCount);
        static bool tryInsert(Vertex landmark, Vertex target, const LabelSet &labelSet,
                              std::vector<std::vector<LabelSet>> &vertexLookup, uint32_t &totalCount);

        void tryInsertNonLandmark(Vertex vertex, Vertex landmark, const LabelSet &labelSet,
                                  std::vector<std::vector<LabelSet>> &vertexLookup, uint32_t &totalCount);

        void tryInsertReachableEntry(Vertex landmark, Vertex vertex, const LabelSet &labelSet, int &reachableIdx);
        void mergeAndFixReachable(Vertex landmark);
        int findReachableEntry(Vertex landmark, const LabelSet &labelSet);
    };
}