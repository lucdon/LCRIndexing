#include <utility/CategorizedStepTimer.hpp>
#include "KLCIndex.hpp"

namespace lcr {
    void KLCIndex::train() {
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

        indices.reserve(expectedCount - labeledGraph.getLabelCount());
        sccGraphs.reserve(expectedCount - labeledGraph.getLabelCount());

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
            allIndex = ReachabilityIndex::create(reachIndexName, std::to_string(reachIndexOptionalParam));

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

    void KLCIndex::createSingleIndex(Label label) {
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

    bool KLCIndex::createIndex(const LabelSet &labelSet) {
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

        return true;
    }

    bool KLCIndex::query(const LCRQuery &query) {
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
        if (reachIndex != indices.end()) {
            return reachIndex->second->query(reachQuery);
        } else if (labels.count() == 1) {
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

        // Fall back to default strategy.
        return defaultStrategy(query);
    }

    QueryResult KLCIndex::queryOnce(const LCRQuery &query) {
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
        if (reachIndex != indices.end()) {
            return reachIndex->second->query(reachQuery) ? QR_Reachable : QR_NotReachable;
        } else if (labels.count() == 1) {
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

        // Fall back to default strategy.
        return QR_MaybeReachable;
    }

    bool KLCIndex::queryBelowCombinations(const ReachQuery &query, const LabelSet &labelSet, const std::vector<Label>& labels) {
        LabelSet toCheckSet(getLabelCount());
        return queryForCombination(query, toCheckSet, labels, 0, labels.size() - 1, 0);
    }

    bool
    KLCIndex::queryForCombination(const ReachQuery &reachQuery, LabelSet &labelSet, const std::vector<Label> &labels,
                                  uint32_t start, uint32_t end, uint32_t index) {
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

    bool KLCIndex::defaultStrategy(const LCRQuery &query) {
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

    size_t KLCIndex::indexSize() const {
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

//         Idea 1:
//         For every non-empty label join the graph with another non-empty label.
//         Keep track of distinct added edges, if there are no edges added, skip the indexing and point to the previous index.
//         Use the previous joined layer to join to the next layer.
//         Thus only indexing the label combinations that actually occur.
//
//         Idea 2: do the same but then only store the graphs that are in the top x % widely different graphs.
//         I.e. the graphs that have the most newly added edges.
//         Here we cannot point to the previous index in case it did decide to not add.
//         Since it might still be slightly different.
//         However we could still check the previous index since it might still return true for that index.
//
//         Idea 3: see if either of the above ideas work and if one does, try to translate it to the BloomPathIndex.
//         (BloomPathIndex is the index where we create a bloom filter based on the first label at a node)
//         This could then be scaled to something like this:
//         Create a bloom filter for the first k labels that occur from a given node.
//         Most notable question would be: how do we access the bloom filters in a performant way?
//         Might be extended with idea 4, to prevent an explosion?
//
//         Idea 4: for graphs that have many overlapping labels
//         I. e. given the labels isNephew and isFriend most edges that are labeled as nephew are also labeled as friends.
//         So create a reduced label isNephewAndFriend which might significantly reduce the explosion in labels for real world use cases.
//
//         Idea 5: Make KCL query aware, so first evaluate on some queries, based on the queries, generate the label permutations to use
//         Also probably include some lower bound so for example, generate up to k=2 and then for k>2 only include label permutations
//         that are found in the queries.
//
//         Idea 6: Would it be possible to improve the fallback situation? Since a lot of permutations have at that point already been
//         checked?
//         - Since we already know that any label combination with less then k labels is unreachable, we could store the max reachable label count
//           for a vertex, if the reachable label count < k then we know that if it was in that path, we must have already found it and can thus prune the
//           sub path.
//           Problem: how to keep track on such a statistic while querying, especially with min-label set.
//
//         Idea 7: Partition the graph, create a klc index on each partition of the graph?