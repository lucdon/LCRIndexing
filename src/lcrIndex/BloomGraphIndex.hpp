#pragma once

#include "Index.hpp"
#include "dataStructures/BloomFilter.hpp"

namespace lcr {
    class BloomGraphIndex : public Index {
    private:
        std::string indexName;

        uint32_t labelSize;
        uint32_t labelBitSize;

        std::vector<BloomFilter> bloomFilters;

    public:
        explicit BloomGraphIndex(uint32_t k) {
            labelSize = k;
            labelBitSize = k * 32;

            indexName = "Bloom Graph Labeling k=" + std::to_string(k);
        }

        void train() override;
        bool query(const LCRQuery &query) override;

        [[nodiscard]] size_t indexSize() const override;
        [[nodiscard]] const std::string &getName() const override {
            return indexName;
        }

    private:
        void createIndexForVertex(VertexQueue &queue, Vertex vertex, std::vector<std::vector<LabelSet>> &vertexLookup);

        bool tryInsert(Vertex vertex, Vertex target, const LabelSet &labelSet,
                       std::vector<std::vector<LabelSet>> &vertexLookup);
    };
}