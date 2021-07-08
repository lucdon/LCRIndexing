#include <utility/Format.hpp>
#include "BFLPathIndex.hpp"

namespace lcr {
    void BFLPathIndex::forwardBFS(const DiGraph &graph, Vertex source, std::vector<BloomFilter> &bloomFilters) {
        boost::dynamic_bitset<> currentVisited(graph.getVertexCount());

        std::deque<Vertex> queue;
        Vertex prevSource;

        currentVisited[source] = true;
        queue.emplace_back(source);

        while (!queue.empty()) {
            prevSource = source;
            source = queue.front();
            queue.pop_front();

            if (prevSource != source) {
                bloomFilters[source] |= bloomFilters[prevSource];
            }

            for (auto vertex : graph.getConnected(source)) {
                if (!currentVisited[vertex]) {
                    currentVisited[vertex] = true;
                    queue.emplace_back(vertex);
                }
            }
        }
    }

    void BFLPathIndex::reverseBFS(const DiGraph &graph, Vertex source, std::vector<BloomFilter> &bloomFilters) {
        boost::dynamic_bitset<> currentVisited(graph.getVertexCount());

        std::deque<Vertex> queue;
        Vertex prevSource;

        currentVisited[source] = true;
        queue.emplace_back(source);

        while (!queue.empty()) {
            prevSource = source;
            source = queue.front();
            queue.pop_front();

            if (prevSource != source) {
                bloomFilters[source] |= bloomFilters[prevSource];
            }

            for (auto vertex : graph.getReverseConnected(source)) {
                if (!currentVisited[vertex]) {
                    currentVisited[vertex] = true;
                    queue.emplace_back(vertex);
                }
            }
        }
    }

    void BFLPathIndex::train() {
        auto &labeledGraph = getGraph();
        auto sccGraph = tarjanSCC(labeledGraph, true);

        auto &componentGraph = sccGraph->getComponentGraph();

        // Step 1: create bloom filter per component.
        std::vector<BloomFilter> toBloomFilters(componentGraph.getVertexCount());
        std::vector<BloomFilter> fromBloomFilters(componentGraph.getVertexCount());

        for (auto component = 0u; component < componentGraph.getVertexCount(); component++) {
            toBloomFilters[component].setup(labelBitSize);
            fromBloomFilters[component].setup(labelBitSize);

            for (auto vertex : sccGraph->getVerticesForComponent(component)) {
                toBloomFilters[component].add(vertex);
                fromBloomFilters[component].add(vertex);
            }
        }

        // For every reachable component, merge bloom filters.
        for (auto component = 0u; component < componentGraph.getVertexCount(); component++) {
            if (componentGraph.getReverseConnected(component).empty()) {
                forwardBFS(componentGraph, component, fromBloomFilters);
            }

            if (componentGraph.getConnected(component).empty()) {
                reverseBFS(componentGraph, component, toBloomFilters);
            }
        }

        toFilters.resize(labeledGraph.getVertexCount());
        fromFilters.resize(labeledGraph.getVertexCount());

        for (auto vertex = 0u; vertex < labeledGraph.getVertexCount(); vertex++) {
            //toFilters[vertex].reserve(labeledGraph.getLabelCount());
            //fromFilters[vertex].reserve(labeledGraph.getLabelCount());

            auto revIt = labeledGraph.getReverseConnected(vertex);

            while (revIt.next()) {
                auto &edge = *revIt;

                if (fromFilters[vertex][edge.label].empty()) {
                    fromFilters[vertex][edge.label].setup(labelBitSize);
                }

                fromFilters[vertex][edge.label] |= fromBloomFilters[sccGraph->getComponentIndex(edge.target)];
            }

            auto it = labeledGraph.getConnected(vertex);

            while (it.next()) {
                auto &edge = *it;

                if (toFilters[vertex][edge.label].empty()) {
                    toFilters[vertex][edge.label].setup(labelBitSize);
                }

                toFilters[vertex][edge.label] |= toBloomFilters[sccGraph->getComponentIndex(edge.target)];
            }
        }
    }

    QueryResult BFLPathIndex::queryOnce(const LCRQuery &query) {
        auto source = query.source;
        auto target = query.target;

        auto sourcePos = boost::hash_value(source) % labelBitSize;
        auto targetPos = boost::hash_value(target) % labelBitSize;

        bool outgoingFound = false;
        bool incomingFound = false;

        for (auto label : query.labels) {
            if (!outgoingFound) {
                auto itTo = toFilters[source].find(label);

                if (itTo != toFilters[source].end() && itTo->second.containsAt(targetPos)) {
                    outgoingFound = true;
                }
            }

            if (!incomingFound) {
                auto itFrom = fromFilters[target].find(label);

                if (itFrom != fromFilters[target].end() && itFrom->second.containsAt(sourcePos)) {
                    incomingFound = true;
                }
            }

            if (incomingFound && outgoingFound) {
                break;
            }
        }

        if (!incomingFound || !outgoingFound) {
            return QR_NotReachable;
        }

        return QR_MaybeReachable;
    }

    bool BFLPathIndex::query(const LCRQuery &query) {
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

        boost::dynamic_bitset<> visited(graph.getVertexCount());
        std::deque<Vertex> queue;

        auto targetPos = boost::hash_value(target) % labelBitSize;

        visited[source] = true;
        queue.emplace_back(source);

        while (!queue.empty()) {
            source = queue.back();
            queue.pop_back();

            bool outgoingFound = false;
            bool incomingFound = false;

            auto sourcePos = boost::hash_value(source) % labelBitSize;

            for (auto label : query.labels) {
                if (!outgoingFound) {
                    auto itTo = toFilters[source].find(label);

                    if (itTo != toFilters[source].end() && itTo->second.containsAt(targetPos)) {
                        outgoingFound = true;
                    }
                }

                if (!incomingFound) {
                    auto itFrom = fromFilters[target].find(label);

                    if (itFrom != fromFilters[target].end() && itFrom->second.containsAt(sourcePos)) {
                        incomingFound = true;
                    }
                }

                if (incomingFound && outgoingFound) {
                    break;
                }
            }

            if (!incomingFound || !outgoingFound) {
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

    size_t BFLPathIndex::indexSize() const {
        size_t size = 0;

        for (auto vertex = 0; vertex < getVertexCount(); vertex++) {
            for (auto label = 0; label < getLabelCount(); label++) {
                if (toFilters[vertex].count(label) != 0) {
                    size += toFilters[vertex].at(label).sizeInBytes();
                }

                if (fromFilters[vertex].count(label) != 0) {
                    size += fromFilters[vertex].at(label).sizeInBytes();
                }
            }
        }

        return size;
    }
}