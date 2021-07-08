#pragma once

#include "Index.hpp"
#include "dataStructures/BloomFilter.hpp"

namespace lcr {
    class BloomPathIndex : public Index {
    private:
        std::string indexName;

        uint32_t labelSize;
        uint32_t labelBitSize;

        std::vector<std::vector<BloomFilter>> toFilters;
        std::vector<std::vector<BloomFilter>> fromFilters;

    public:
        explicit BloomPathIndex(uint32_t k) {
            labelSize = k;
            labelBitSize = 32 * k;

            indexName = "Bloom First Path Labeling k=" + std::to_string(k);
        }

        void train() override;
        bool query(const LCRQuery &query) override;

        [[nodiscard]] size_t indexSize() const override;
        [[nodiscard]] const std::string &getName() const override {
            return indexName;
        }

    private:
        void forwardBFS(VertexOriginQueue &queue, boost::dynamic_bitset<> &visited, Vertex vertex,
                        const boost::dynamic_bitset<> &bfsVisited);
        void reverseBFS(VertexOriginQueue &queue, boost::dynamic_bitset<> &visited, Vertex vertex,
                        const boost::dynamic_bitset<> &bfsVisited);

        void copyToIndex(Vertex vertex, Vertex target, Label label);
        void copyFromIndex(Vertex vertex, Vertex source, Label label);

        void insertToVertex(Vertex vertex, Vertex target, Label label);
        void insertFromVertex(Vertex vertex, Vertex source, Label label);
    };
}