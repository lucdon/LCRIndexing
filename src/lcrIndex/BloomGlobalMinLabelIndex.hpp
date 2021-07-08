#pragma once

#include "Index.hpp"
#include "dataStructures/BloomFilter.hpp"

namespace lcr {
    class BloomGlobalMinLabelIndex : public Index {
    private:
        std::string indexName;

        uint32_t labelSize;
        uint32_t labelBitSize;

        std::vector<std::vector<BloomFilter>> toFilters;

    public:
        explicit BloomGlobalMinLabelIndex(uint32_t k) {
            labelSize = k;
            labelBitSize = 32 * k;

            indexName = "Bloom Min Global Labeling k=" + std::to_string(k);
        }

        void train() override;
        bool query(const LCRQuery &query) override;

        [[nodiscard]] size_t indexSize() const override;
        [[nodiscard]] const std::string &getName() const override {
            return indexName;
        }

    private:
        void forwardBFS(VertexQueue &queue, Vertex vertex, std::vector<std::vector<LabelSet>> &vertexLookup,
                        const std::vector<uint32_t> &labelFrequencies);

        bool tryInsertToVertex(Vertex vertex, Vertex target, std::vector<std::vector<LabelSet>> &vertexLookup,
                               const LabelSet &labelSet, const std::vector<uint32_t> &labelFrequencies);
    };
}