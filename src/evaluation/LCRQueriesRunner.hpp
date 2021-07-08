#pragma once

#include "lcrIndex/Index.hpp"
#include "utility/Limit.hpp"

namespace lcr {
    class QueriesRunner {

    private:
        std::unique_ptr<Index> controlIndex = nullptr;
        std::vector<std::unique_ptr<Index>> indices;

        std::unique_ptr<Limit> limit = nullptr;

    public:

        void setControlIndex(std::unique_ptr<Index> &&index) {
            controlIndex = std::move(index);
        }

        void addIndex(std::unique_ptr<Index> &&index) {
            indices.emplace_back(std::move(index));
        }

        void setLimit(std::unique_ptr<Limit> &&lim) {
            limit = std::move(lim);
        }

        void run(std::string &graphFile, const std::vector<std::string>& queryFiles);
    };
}