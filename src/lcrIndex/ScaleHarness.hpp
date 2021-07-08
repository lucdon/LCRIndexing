#pragma once

#include <utility>

#include "Index.hpp"

namespace lcr {
    class ScaleHarness : public Index {
    private:
        std::vector<Label> primaryLabelMapping;
        std::vector<Label> secondaryLabelMapping;

        uint32_t numMostFrequentLabels;

        std::unique_ptr<Index> primaryIndex = nullptr;
        std::unique_ptr<Index> secondaryIndex = nullptr;

        std::string indexName = "Scale Harness w=";
        std::string createdIndexName;
        std::vector<std::string> createdIndexParams;

        boost::dynamic_bitset<> landmarked;

    public:

        explicit ScaleHarness(const std::string& indexName, std::vector<std::string> params,
                              uint32_t numMostFrequentLabels) : createdIndexName(indexName),
                                                                createdIndexParams(std::move(params)),
                                                                numMostFrequentLabels(numMostFrequentLabels) {
            this->indexName += std::to_string(numMostFrequentLabels) + " idx=" + indexName;
        }

        void train() override;
        bool query(const LCRQuery &query) override;

        [[nodiscard]] size_t indexSize() const override;
        [[nodiscard]] const std::string &getName() const override { return indexName; }

    private:
        bool defaultStrategy(const LCRQuery &query, bool isPrimaryPossible, const LCRQuery &primaryQuery,
                             const LCRQuery &secondaryQuery);
    };
}
