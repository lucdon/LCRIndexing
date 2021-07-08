#include <utility/CategorizedStepTimer.hpp>
#include "KLCFreqIndex.hpp"

namespace lcr {
    void KLCFreqIndex::train() {
        auto &labeledGraph = getGraph();
        uint64_t labelCount = labeledGraph.getLabelCount();

        singleLabelIndices.resize(labeledGraph.getLabelCount());

        for (auto label = 0u; label < labeledGraph.getLabelCount(); label++) {
            createSingleIndex(label);
        }

        std::queue<std::tuple<LabelSet, uint32_t, uint32_t>> queue;
        queue.emplace(labelCount, 0, 0);

        auto expectedCount = calculateCombinationsUpToK(maxCombinations, labelCount);

        std::vector<std::pair<uint32_t, Label>> numEdgesByLabel;
        labelDistribution(labeledGraph, numEdgesByLabel);

        aboveLookup.reserve(maxAboveCombinations);

        indices.reserve(expectedCount + maxAboveCombinations - labeledGraph.getLabelCount());
        sccGraphs.reserve(expectedCount + maxAboveCombinations);

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

                createIndex(labelSet, false);
                continue;
            }

            if (count == maxCombinations) {
                if (labelSet.none()) {
                    continue;
                }

                createIndex(labelSet, false);
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
            allIndex = ReachabilityIndex::create("BFL", "4");

            MergedGraphStats stats;
            auto graph = mergeGraphForLabels(getGraph(), labelSet, stats);

            allSccGraph = tarjanSCC(*graph);
            allIndex->setGraph(allSccGraph.get());

            allIndex->train();

            if (allIndex->canDiscardComponentGraph()) {
                allSccGraph->clearComponentGraph();
            }

            std::vector<Vertex> order;
            vertexOrderByDegree(*graph, order);

            std::stack<VertexLabelSet> vertexQueue;
            VertexLabelSetVisitedSet visited;
            std::unordered_map<LabelSet, uint32_t> frequencyMap;
            uint32_t totalSetCount = 0;

            for (auto ordered : order) {
                uint32_t setCount = 0;
                countFrequencies(vertexQueue, ordered, visited, frequencyMap, setCount);

                totalSetCount += setCount;

                if (totalSetCount > maxTotalSetCount) {
                    break;
                }
            }

            std::vector<std::pair<float, LabelSet>> sorted;
            sorted.reserve(frequencyMap.size());

            for (const auto &pair : frequencyMap) {
                if (pair.first.count() <= maxCombinations) {
                    continue;
                }

                sorted.emplace_back(float(pair.second) * float(pair.first.count()), pair.first);
            }

            std::sort(sorted.begin(), sorted.end(), std::greater<>());

            for (auto i = 0; i < maxAboveCombinations && i < sorted.size(); i++) {
                auto &best = sorted[i];

                createIndex(best.second, true);
            }
        }
    }

    void
    KLCFreqIndex::countFrequencies(std::stack<VertexLabelSet> &queue, Vertex vertex, VertexLabelSetVisitedSet &visited,
                                   std::unordered_map<LabelSet, uint32_t> &frequencyMap, uint32_t &setCount) {
        auto &graph = getGraph();

        queue.emplace(vertex, graph.getLabelCount());
        VertexLabelSet current(vertex, graph.getLabelCount());

        while (!queue.empty()) {
            current = queue.top();
            queue.pop();

            auto pair = std::make_pair<>(current.first, current.second);

            if (visited.count(pair) != 0) {
                continue;
            }

            auto count = current.second.count();
            visited.emplace(current.first, current.second);

            if (count >= minLabelsAboveCombinations) {
                auto ptr = frequencyMap.find(current.second);

                if (ptr != frequencyMap.end()) {
                    ptr->second++;
                } else {
                    frequencyMap.emplace(current.second, 1);
                }

                setCount++;

                if (setCount > maxPerVertexSetCount) {
                    return;
                }
            }

            if (count > maxLabelsAboveCombinations) {
                continue;
            }

            auto it = graph.getConnected(current.first);

            while (it.next()) {
                auto &edge = *it;

                LabelSet labelSet(graph.getLabelCount());

                labelSet[edge.label] = true;
                labelSet |= current.second;

                queue.emplace(edge.target, labelSet);
            }
        }
    }

    void KLCFreqIndex::createSingleIndex(Label label) {
        if (singleLabelIndices[label] != nullptr) {
            // Already indexed.
            return;
        }

        auto &labeledGraph = getGraph();

        LabelSet labelSet(labeledGraph.getLabelCount());
        labelSet[label] = true;

        MergedGraphStats stats;
        auto graph = mergeGraphForLabels(labeledGraph, labelSet, stats);

        singleLabelIndices[label] = ReachabilityIndex::create(reachIndexName, std::to_string(reachIndexOptionalParam));
        auto &index = singleLabelIndices[label];

        auto &sccGraph = sccGraphs.emplace_back();
        sccGraph = std::move(tarjanSCC(*graph));

        index->setGraph(sccGraph.get());

        index->train();

        if (index->canDiscardComponentGraph()) {
            sccGraph->clearComponentGraph();
        }
    }

    bool KLCFreqIndex::createIndex(const LabelSet &labelSet, bool isAbove) {
        if (labelSet.count() <= 1) {
            return false;
        }

        if (labelSet.all()) {
            return false;
        }

        auto &labeledGraph = getGraph();

        MergedGraphStats stats;
        auto graph = mergeGraphForLabels(labeledGraph, labelSet, stats);

        // Only index the label set if it gives a significant win.
        // Otherwise it will be found by a previous combination.
        if (std::abs(stats.increasePercentage) <= 0) {
            return false;
        }

        auto &index = indices[labelSet];
        index = ReachabilityIndex::create(reachIndexName, std::to_string(reachIndexOptionalParam));

        auto &sccGraph = sccGraphs.emplace_back();
        sccGraph = std::move(tarjanSCC(*graph));

        index->setGraph(sccGraph.get());

        index->train();

        if (index->canDiscardComponentGraph()) {
            sccGraph->clearComponentGraph();
        }

        if (isAbove) {
            aboveLookup.emplace_back(labelSet, index.get());
        }

        return true;
    }

    bool KLCFreqIndex::query(const LCRQuery &query) {
        auto source = query.source;
        auto target = query.target;

        if (source == target) {
            return true;
        }

        auto &labels = query.labelSet;

        if (labels.none()) {
            return false;
        }

        ReachQuery reachQuery(query.source, query.target);
        auto reachIndex = indices.find(query.labelSet);

        // If there is an exact match, use that index.
        if (reachIndex != indices.end() && reachIndex->second->getName()[0] != 'B') {
            return reachIndex->second->query(reachQuery);
        } else if (query.labels.size() == 1) {
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
        if (!allIndex->query(reachQuery)) {
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

        // Go over all combinations that could match.
        if (queryBelowCombinations(reachQuery, query.labelSet, query.labels)) {
            return true;
        }

        // Go over all combinations that could match.
        ReachabilityIndex *bestBound = allIndex.get();

        // If it is 1 then there exists an above combination that did not reach
        if (queryAboveCombinations(reachQuery, query.labelSet, bestBound) == -1) {
            return false;
        }

        // Fall back to default strategy.
        return defaultStrategy(query, bestBound);
    }

    QueryResult KLCFreqIndex::queryOnce(const LCRQuery &query) {
        auto source = query.source;
        auto target = query.target;

        if (source == target) {
            return QR_Reachable;
        }

        auto &labels = query.labelSet;

        if (labels.none()) {
            return QR_NotReachable;
        }

        ReachQuery reachQuery(query.source, query.target);
        auto reachIndex = indices.find(query.labelSet);

        // If there is an exact match, use that index.
        if (reachIndex != indices.end() && reachIndex->second->getName()[0] != 'B') {
            return reachIndex->second->query(reachQuery) ? QR_Reachable : QR_NotReachable;
        } else if (query.labels.size() == 1) {
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
        if (!allIndex->query(reachQuery)) {
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

        // Go over all combinations that could match.
        if (queryBelowCombinations(reachQuery, query.labelSet, query.labels)) {
            return QR_Reachable;
        }

        // Go over all combinations that could match.
        ReachabilityIndex *bestBound = allIndex.get();

        // If it is 1 then there exists an above combination that did not reach
        if (queryAboveCombinations(reachQuery, query.labelSet, bestBound) == -1) {
            return QR_NotReachable;
        }

        // Fall back to default strategy.
        return QR_MaybeReachable;
    }

    bool KLCFreqIndex::queryBelowCombinations(const ReachQuery &query, const LabelSet &labelSet,
                                              const std::vector<Label> &labels) {
        LabelSet toCheckSet(getLabelCount());
        return queryForCombination(query, toCheckSet, labels, 0, labels.size() - 1, 0);
    }

    int8_t KLCFreqIndex::queryAboveCombinations(const ReachQuery &query, const LabelSet &labelSet,
                                                ReachabilityIndex *&bestBound) {
        uint32_t bestCount = getLabelCount();

        for (auto &indexPair : aboveLookup) {
            if (labelSet.is_subset_of(indexPair.first)) {
                if (!indexPair.second->queryOnce(query)) {
                    return -1;
                } else {
                    auto count = indexPair.first.count();

                    if (count < bestCount) {
                        bestCount = count;
                        bestBound = indexPair.second;
                    }
                }
            }
        }

        return 0;
    }

    bool KLCFreqIndex::queryForCombination(const ReachQuery &reachQuery, LabelSet &labelSet,
                                           const std::vector<Label> &labels, uint32_t start, uint32_t end,
                                           uint32_t index) {
        if (index == maxCombinations) {
            auto reachIndexIt = indices.find(labelSet);

            if (reachIndexIt != indices.end()) {
                return reachIndexIt->second->query(reachQuery);
            }

            // Nothing found here.
            return false;
        }

        for (uint32_t i = start; i <= end && end - i + 1 >= maxCombinations - index; i++) {
            labelSet[labels[i]] = true;

            if (queryForCombination(reachQuery, labelSet, labels, i + 1, end, index + 1)) {
                return true;
            }

            labelSet[labels[i]] = false;
        }

        return false;
    }

    bool KLCFreqIndex::defaultStrategy(const LCRQuery &query, ReachabilityIndex *&bestBound) {
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

            if (!bestBound->queryOnce(reachQuery)) {
                continue;
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

    size_t KLCFreqIndex::indexSize() const {
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
