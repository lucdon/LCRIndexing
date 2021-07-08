#include <utility/CategorizedStepTimer.hpp>
#include "ScaleHarness.hpp"

namespace lcr {
    void ScaleHarness::train() {
        auto &graph = getGraph();

        boost::dynamic_bitset<> visited(graph.getVertexCount());

        if (graph.getLabelCount() <= numMostFrequentLabels) {
            std::vector<Label> labels(graph.getLabelCount());

            for (Label label = 0u; label < graph.getLabelCount(); label++) {
                labels[label] = label;
            }

            primaryIndex = Index::create(createdIndexName, createdIndexParams);
            primaryIndex->setGraph(const_cast<LabeledEdgeGraph *>(&graph));
            primaryIndex->train();
        } else {
            std::vector<Label> labelOrder;
            orderLabelsByFrequency(graph, labelOrder);

            LabelSet labelSet(graph.getLabelCount());

            landmarked.resize(graph.getVertexCount());

            std::vector<Vertex> vertexOrder;
            vertexOrderByDegree(graph, vertexOrder);

            for(auto i = 0u; i < 8 * sqrt(graph.getVertexCount()); i++) {
                landmarked[i] = true;
            }

            primaryLabelMapping.resize(graph.getLabelCount());
            std::fill(primaryLabelMapping.begin(), primaryLabelMapping.end(), std::numeric_limits<uint32_t>::max());

            for (auto i = 0; i < numMostFrequentLabels; i++) {
                labelSet[labelOrder[i]] = true;
                primaryLabelMapping[labelOrder[i]] = i;
            }

            {
                auto primaryGraph = splitGraph(graph, labelSet, primaryLabelMapping);

                primaryIndex = Index::create(createdIndexName, createdIndexParams);
                primaryIndex->setGraph(primaryGraph.get());
                primaryIndex->train();
            }

            visited.reset();
            secondaryLabelMapping.resize(graph.getLabelCount());

            {
                auto virtualLabelGraph = createVirtualLabelGraph(graph, labelOrder, numMostFrequentLabels / 2,
                                                                 numMostFrequentLabels / 2, secondaryLabelMapping);

                secondaryIndex = Index::create(createdIndexName, createdIndexParams);
                secondaryIndex->setGraph(virtualLabelGraph.get());
                secondaryIndex->train();
            }
        }
    }

    bool ScaleHarness::query(const LCRQuery &query) {
        auto &graph = getGraph();

        auto source = query.source;
        auto target = query.target;

        if (source == target) {
            return true;
        }

        auto &labels = query.labelSet;

        if (labels.none()) {
            return false;
        }

        if (graph.getLabelCount() <= numMostFrequentLabels) {
            return primaryIndex->query(query);
        }

        LabelSet labelSet(numMostFrequentLabels);
        std::vector<Label> primaryLabels;
        bool fullyIncluded = true;
        bool included = false;

        for (auto label : query.labels) {
            if (primaryLabelMapping[label] != std::numeric_limits<uint32_t>::max()) {
                included = true;
                labelSet[primaryLabelMapping[label]] = true;
                primaryLabels.emplace_back(primaryLabelMapping[label]);
            } else {
                fullyIncluded = false;
            }
        }

        LCRQuery primaryQuery;
        primaryQuery.labelSet = labelSet;
        primaryQuery.labels = primaryLabels;
        primaryQuery.source = source;
        primaryQuery.target = target;

        if (fullyIncluded) {
            auto result = primaryIndex->queryOnce(primaryQuery);

            if (result == QR_NotReachable) {
                return false;
            }

            if (result == QR_Reachable) {
                return true;
            }
        } else {
            if (included && primaryIndex->queryOnce(primaryQuery) == QR_Reachable) {
                return true;
            }
        }

        LabelSet secondaryLabelSet(numMostFrequentLabels);
        std::vector<Label> secondaryLabels;

        for (auto label : query.labels) {
            secondaryLabelSet[secondaryLabelMapping[label]] = true;
            secondaryLabels.emplace_back(secondaryLabelMapping[label]);
        }

        LCRQuery secondaryQuery;
        secondaryQuery.labelSet = secondaryLabelSet;
        secondaryQuery.labels = secondaryLabels;
        secondaryQuery.source = source;
        secondaryQuery.target = target;

        if (secondaryIndex->queryOnce(secondaryQuery) == QR_NotReachable) {
            return false;
        }

        return defaultStrategy(query, included, primaryQuery, secondaryQuery);
    }

    bool ScaleHarness::defaultStrategy(const LCRQuery &query, bool isPrimaryPossible, const LCRQuery &primaryQuery,
                                       const LCRQuery &secondaryQuery) {
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

            if(landmarked[source]) {
                if (secondaryIndex->queryOnceRecursive(secondaryQuery) == QR_NotReachable) {
                    continue;
                }

                if (isPrimaryPossible && primaryIndex->queryOnceRecursive(primaryQuery) == QR_Reachable) {
                    return true;
                }
            }

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

    size_t ScaleHarness::indexSize() const {
        size_t size = 0;

        size += primaryIndex->indexSize();

        if (getGraph().getLabelCount() <= numMostFrequentLabels) {
            return size;
        }

        size += secondaryIndex->indexSize();

        return size;
    }
}