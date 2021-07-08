#pragma once

#include <dataStructures/BloomFilter.hpp>
#include "Index.hpp"

namespace lcr {
    class BFLPathIndex : public Index {
    private:
        std::string indexName;

        uint32_t labelSize;
        uint32_t labelBitSize;

        std::vector<std::map<uint32_t, BloomFilter>> toFilters = { };
        std::vector<std::map<uint32_t, BloomFilter>> fromFilters = { };

    public:
        explicit BFLPathIndex(uint32_t k) {
            labelSize = k;
            labelBitSize = k * 32;

            indexName = "Bloom Filter Path Labeling k=" + std::to_string(k);
        }

        void train() override;
        bool query(const LCRQuery &query) override;

        virtual QueryResult queryOnce(const LCRQuery &q) override;

        [[nodiscard]] size_t indexSize() const override;

        [[nodiscard]] const std::string &getName() const override {
            return indexName;
        }

    private:
        void forwardBFS(const DiGraph &graph, Vertex source, std::vector<BloomFilter>& bloomFilters);
        void reverseBFS(const DiGraph &graph, Vertex source, std::vector<BloomFilter>& bloomFilters);
    };
}