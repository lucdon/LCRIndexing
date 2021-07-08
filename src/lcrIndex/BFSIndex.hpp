#pragma once

#include "Index.hpp"

namespace lcr {
    class BFSIndex : public Index {
    private:
        inline static const std::string indexName = "BFS";

    public:
        void train() override { }
        bool query(const LCRQuery &query) override;

        [[nodiscard]] size_t indexSize() const override { return 0; }
        [[nodiscard]] const std::string &getName() const override { return indexName; }
    };
}