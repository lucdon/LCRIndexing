#include "ALCIndex.hpp"

namespace lcr {
    void ALCIndex::train() {
        std::queue<std::pair<LabelSet, uint32_t>> stack;

        auto labelCount = getGraph().getLabelCount();
        stack.emplace(labelCount, 0);

        if (labelCount > 18) {
            std::cerr << "too many labels. ACL only supports up to 18 labels. Which already creates 262.143 sub-indexes"
                      << std::fatal;
        }

        uint64_t expectedCount = (1ull << uint64_t(labelCount)) - 1;

        indices.reserve(expectedCount);
        sccGraphs.reserve(expectedCount);

        while (!stack.empty()) {
            auto current = stack.front();
            stack.pop();

            auto &labelSet = current.first;
            auto i = current.second;

            // Do something with combination
            if (i >= labelCount) {
                if (labelSet.none()) {
                    continue;
                }

                createIndex(labelSet);
                continue;
            }

            LabelSet next(labelSet.size());
            next |= labelSet;
            next[i] = true;

            stack.emplace(labelSet, i + 1);
            stack.emplace(next, i + 1);
        }
    }

    void ALCIndex::createIndex(const LabelSet &labelSet) {
        auto &index = indices[labelSet];
        index = ReachabilityIndex::create(reachIndexName, std::to_string(reachIndexOptionalParam));

        auto &labeledGraph = getGraph();
        MergedGraphStats stats;
        auto graph = mergeGraphForLabels(labeledGraph, labelSet, stats);
        sccGraphs[labelSet] = tarjanSCC(*graph);
        auto& sccGraph = sccGraphs[labelSet];

        index->setGraph(sccGraph.get());

        index->train();

        if (index->canDiscardComponentGraph()) {
            sccGraph->clearComponentGraph();
        }
    }

    bool ALCIndex::query(const LCRQuery &query) {
        ReachQuery reachQuery(query.source, query.target);
        return indices.at(query.labelSet)->query(reachQuery);
    }

    size_t ALCIndex::indexSize() const {
        size_t size = 0u;

        for (auto &index : indices) {
            size += index.second->indexSize();
        }

        for (auto &sccGraph : sccGraphs) {
            size += sccGraph.second->getSizeInBytes();
        }

        return size;
    }
}
