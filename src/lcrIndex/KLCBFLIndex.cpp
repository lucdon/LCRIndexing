#include <utility/CategorizedStepTimer.hpp>
#include "KLCBFLIndex.hpp"

namespace lcr {
    void KLCBFLIndex::train() {
        auto &perLabelGraph = getGraph();
        uint64_t labelCount = perLabelGraph.getLabelCount();

        singleLabelIndices.resize(perLabelGraph.getLabelCount());

        for (auto label = 0u; label < perLabelGraph.getLabelCount(); label++) {
            createSingleIndex(label);
        }

        std::queue<std::tuple<LabelSet, uint32_t, uint32_t>> queue;
        queue.emplace(labelCount, 0, 0);

        auto expectedCount = calculateCombinationsUpToK(maxCombinations, labelCount);

        std::vector<std::pair<uint32_t, Label>> numEdgesByLabel;
        labelDistribution(perLabelGraph, numEdgesByLabel);

        indices.reserve(expectedCount);
        sccGraphs.reserve(expectedCount);

        while (!queue.empty()) {
            auto current = queue.front();
            queue.pop();

            auto &labelSet = std::get<0>(current);
            auto i = std::get<1>(current);
            auto count = std::get<2>(current);

            // Do something with combination
            if (i >= labelCount) {
                if (labelSet.none()) {
                    continue;
                }

                createIndex(labelSet);
                continue;
            }

            if (count == maxCombinations) {
                if (labelSet.none()) {
                    continue;
                }

                createIndex(labelSet);
                continue;
            }

            LabelSet next(labelSet.size());
            next |= labelSet;
            next[i] = true;

            queue.emplace(labelSet, i + 1, count);
            queue.emplace(next, i + 1, count + 1);
        }

        if (maxCombinations < labelCount) {
            LabelSet labelSet(labelCount);
            labelSet.set();

            // Create for all labels.
            allIndex = ReachabilityIndex::create("PLL");

            MergedGraphStats stats;
            auto graph = mergeGraphForLabels(getGraph(), labelSet, stats);

            allSccGraph = tarjanSCC(*graph);
            allIndex->setGraph(allSccGraph.get());

            allIndex->train();

            if (allIndex->canDiscardComponentGraph()) {
                allSccGraph->clearComponentGraph();
            }
        }
    }

    void KLCBFLIndex::createSingleIndex(Label label) {
        if (singleLabelIndices[label] != nullptr) {
            // Already indexed.
            return;
        }

        auto &labeledGraph = getGraph();

        LabelSet labelSet(labeledGraph.getLabelCount());
        labelSet[label] = true;

        MergedGraphStats stats;
        auto graph = mergeGraphForLabels(labeledGraph, labelSet, stats);

        singleLabelIndices[label] = ReachabilityIndex::create("pll");
        auto &index = singleLabelIndices[label];

        auto &sccGraph = sccGraphs.emplace_back();
        sccGraph = std::move(tarjanSCC(*graph));

        index->setGraph(sccGraph.get());

        index->train();

        if (index->canDiscardComponentGraph()) {
            sccGraph->clearComponentGraph();
        }
    }

    bool KLCBFLIndex::createIndex(const LabelSet &labelSet) {
        if (labelSet.count() <= 1) {
            return false;
        }

        if (labelSet.all()) {
            return false;
        }

        auto &labeledGraph = getGraph();

        MergedGraphStats stats;
        auto graph = mergeGraphForLabels(labeledGraph, ~labelSet, stats);

        // Only index the label set if it gives a significant win.
        // Otherwise it will be found by a previous combination.
        if (std::abs(stats.increasePercentage) <= 0) {
            return false;
        }

        auto &indexPair = indices.emplace_back();
        auto &index = indexPair.second;

        indexPair.first = ~labelSet;
        index = ReachabilityIndex::create("bfl-once", "4");

        auto &sccGraph = sccGraphs.emplace_back();
        sccGraph = std::move(tarjanSCC(*graph));

        index->setGraph(sccGraph.get());

        index->train();

        if (index->canDiscardComponentGraph()) {
            sccGraph->clearComponentGraph();
        }

        return true;
    }

    bool KLCBFLIndex::query(const LCRQuery &query) {
        auto source = query.source;
        auto target = query.target;

        if (source == target) {
            return true;
        }

        auto &labels = query.labelSet;
        auto labelCount = labels.count();

        if (labelCount == 0) {
            return false;
        }

        ReachQuery reachQuery(query.source, query.target);

        // If there is an exact match, use that index.
        if (labelCount == 1) {
            // Otherwise if it is a single label, query the single label indices.
            for (auto label = 0u; label < labels.size(); label++) {
                if (!labels[label]) {
                    continue;
                }

                if (singleLabelIndices[label]->query(reachQuery)) {
                    return true;
                }

                return false;
            }
        }

        // If not even reachable in the full graph, we can stop early.
        if (allIndex != nullptr && !allIndex->query(reachQuery)) {
            return false;
        }

        // It was reachable in the all graph and all labels are set.
        if (labels.all()) {
            return true;
        }

        for (auto label : query.labels) {
            // If found in a single label, no need to continue.
            if (singleLabelIndices[label]->query(reachQuery)) {
                return true;
            }
        }

        // Find all indexes that are able to help prune the search.
        std::vector<ReachabilityIndex *> reachIndexes;

        gatherReachIndexes(reachQuery, query.labelSet, reachIndexes);

        // Use bfs and the reachIndexes to prune the search.
        return defaultStrategy(query, reachIndexes);
    }

    QueryResult KLCBFLIndex::queryOnce(const LCRQuery &query) {
        auto source = query.source;
        auto target = query.target;

        if (source == target) {
            return QR_Reachable;
        }

        auto &labels = query.labelSet;
        auto labelCount = labels.count();

        if (labelCount == 0) {
            return QR_NotReachable;
        }

        ReachQuery reachQuery(query.source, query.target);

        // If there is an exact match, use that index.
        if (labelCount == 1) {
            // Otherwise if it is a single label, query the single label indices.
            for (auto label = 0u; label < labels.size(); label++) {
                if (!labels[label]) {
                    continue;
                }

                if (singleLabelIndices[label]->query(reachQuery)) {
                    return QR_Reachable;
                }

                return QR_NotReachable;
            }
        }

        // If not even reachable in the full graph, we can stop early.
        if (allIndex != nullptr && !allIndex->query(reachQuery)) {
            return QR_NotReachable;
        }

        // It was reachable in the all graph and all labels are set.
        if (labels.all()) {
            return QR_Reachable;
        }

        for (auto label : query.labels) {
            // If found in a single label, no need to continue.
            if (singleLabelIndices[label]->query(reachQuery)) {
                return QR_Reachable;
            }
        }

        for (auto &pair : indices) {
            if (query.labelSet.is_subset_of(pair.first) && !pair.second->queryOnce(reachQuery)) {
                return QR_NotReachable;
            }
        }

        // Use bfs and the reachIndexes to prune the search.
        return QR_MaybeReachable;
    }

    void KLCBFLIndex::gatherReachIndexes(const ReachQuery &query, const LabelSet &labelSet,
                                         std::vector<ReachabilityIndex *> &reachIndexes) {

        uint32_t max = 2;

        for (auto &pair : indices) {
            if (labelSet.is_subset_of(pair.first)) {
                reachIndexes.emplace_back(pair.second.get());
                max--;
            }

            if (max == 0) {
                break;
            }
        }
    }

    bool KLCBFLIndex::defaultStrategy(const LCRQuery &query, std::vector<ReachabilityIndex *> &reachIndexes) {
        auto &graph = getGraph();

        auto source = query.source;
        auto target = query.target;
        auto &labels = query.labelSet;

        // Default to BFS.
        boost::dynamic_bitset<> visited(graph.getVertexCount());
        std::deque<Vertex> queue;
        queue.emplace_back(source);

        visited[source] = true;

        while (!queue.empty()) {
            source = queue.front();
            queue.pop_front();

            ReachQuery reachQuery(source, target);

            for (auto &index : reachIndexes) {
                if (!index->queryOnce(reachQuery)) {
                    continue;
                }
            }

            auto it = graph.getConnected(source);

            while (it.next()) {
                auto &edge = *it;

                if (!query.labelSet[edge.label]) {
                    continue;
                }

                if (edge.target == target) {
                    return true;
                }

                if (!visited[edge.target]) {
                    visited[edge.target] = true;
                    queue.emplace_back(edge.target);
                }
            }
        }

        return false;
    }

    size_t KLCBFLIndex::indexSize() const {
        size_t size = 0u;

        for (auto &index : singleLabelIndices) {
            size += index->indexSize();
        }

        for (auto &index : indices) {
            size += index.second->indexSize();
        }

        for (auto &sccGraph : sccGraphs) {
            size += sccGraph->getSizeInBytes();
        }

        if (maxCombinations < getLabelCount()) {
            size += allIndex->indexSize();
            size += allSccGraph->getSizeInBytes();
        }

        return size;
    }
}
