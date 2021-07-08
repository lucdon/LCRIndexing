#include "BloomPathIndex.hpp"

namespace lcr {
    void BloomPathIndex::train() {
        auto &graph = getGraph();

        std::vector<Vertex> outOrder;
        std::vector<Vertex> inOrder;
        vertexOrderByDegree(graph, outOrder);
        vertexOrderByDegree(graph, inOrder);

        VertexOriginQueue queue;
        boost::dynamic_bitset<> visited(graph.getVertexCount());
        boost::dynamic_bitset<> bfsVisitedIn(graph.getVertexCount());
        boost::dynamic_bitset<> bfsVisitedOut(graph.getVertexCount());

        toFilters.resize(graph.getVertexCount());
        fromFilters.resize(graph.getVertexCount());

        for (auto i = 0u; i < graph.getVertexCount(); i++) {
            toFilters[i].resize(graph.getLabelCount());
            fromFilters[i].resize(graph.getLabelCount());
        }

        for (auto i = 0u; i < graph.getVertexCount(); i++) {
            auto vertexOut = outOrder[i];
            auto vertexIn = inOrder[i];

            auto it = graph.getConnected(vertexOut);

            while (it.next()) {
                auto &edge = *it;

                if (toFilters[vertexOut][edge.label].empty()) {
                    toFilters[vertexOut][edge.label].setup(labelBitSize);
                }

                visited[vertexOut] = true;
                visited[edge.target] = true;

                queue.emplace(edge.target, edge.label);
                forwardBFS(queue, visited, vertexOut, bfsVisitedOut);
                visited.reset();
            }

            auto revIt = graph.getReverseConnected(vertexIn);

            while (revIt.next()) {
                auto &edge = *revIt;

                if (fromFilters[vertexIn][edge.label].empty()) {
                    fromFilters[vertexIn][edge.label].setup(labelBitSize);
                }

                visited[vertexIn] = true;
                visited[edge.target] = true;

                queue.emplace(edge.target, edge.label);
                reverseBFS(queue, visited, vertexIn, bfsVisitedIn);
                visited.reset();
            }

            bfsVisitedOut[vertexOut] = true;
            bfsVisitedIn[vertexIn] = true;
        }
    }

    void BloomPathIndex::forwardBFS(VertexOriginQueue &queue, boost::dynamic_bitset<> &visited, Vertex origin,
                                    const boost::dynamic_bitset<> &bfsVisited) {
        auto &graph = getGraph();

        while (!queue.empty()) {
            auto current = queue.front();
            queue.pop();

            insertToVertex(origin, current.vertex, current.label);

            if (bfsVisited[current.vertex]) {
                copyToIndex(origin, current.vertex, current.label);
                continue;
            }

            auto it = graph.getConnected(current.vertex);

            while (it.next()) {
                auto &edge = *it;

                if (visited[edge.target]) {
                    continue;
                }

                visited[edge.target] = true;
                queue.emplace(edge.target, current.label);
            }
        }
    }

    void BloomPathIndex::reverseBFS(VertexOriginQueue &queue, boost::dynamic_bitset<> &visited, Vertex origin,
                                    const boost::dynamic_bitset<> &bfsVisited) {
        auto &graph = getGraph();

        while (!queue.empty()) {
            auto current = queue.front();
            queue.pop();

            insertFromVertex(origin, current.vertex, current.label);

            if (bfsVisited[current.vertex]) {
                copyFromIndex(origin, current.vertex, current.label);
                continue;
            }

            auto it = graph.getReverseConnected(current.vertex);

            while (it.next()) {
                auto &edge = *it;

                if (visited[edge.target]) {
                    continue;
                }

                visited[edge.target] = true;
                queue.emplace(edge.target, current.label);
            }
        }
    }

    void BloomPathIndex::copyToIndex(Vertex vertex, Vertex target, Label label) {
        auto &bloomFilter = toFilters[vertex][label];

        for (auto i = 0; i < getGraph().getLabelCount(); i++) {
            if (!toFilters[target][i].empty()) {
                bloomFilter |= toFilters[target][i];
            }
        }
    }

    void BloomPathIndex::copyFromIndex(Vertex vertex, Vertex source, Label label) {
        auto &bloomFilter = fromFilters[vertex][label];

        for (auto i = 0; i < getGraph().getLabelCount(); i++) {
            if (!fromFilters[source][i].empty()) {
                bloomFilter |= fromFilters[source][i];
            }
        }
    }

    void BloomPathIndex::insertToVertex(Vertex vertex, Vertex target, Label label) {
        if (vertex == target) {
            return;
        }

        toFilters[vertex][label].add(target);
    }

    void BloomPathIndex::insertFromVertex(Vertex vertex, Vertex source, Label label) {
        if (vertex == source) {
            return;
        }

        fromFilters[vertex][label].add(source);
    }

    bool BloomPathIndex::query(const LCRQuery &query) {
        auto &graph = getGraph();

        auto source = query.source;
        auto target = query.target;

        if (source == target) {
            return true;
        }

        auto &labelSet = query.labelSet;

        if (labelSet.none()) {
            return false;
        }

        boost::dynamic_bitset<> visited(graph.getVertexCount());
        std::deque<Vertex> queue;
        queue.emplace_back(source);

        visited[source] = true;

        while (!queue.empty()) {
            source = queue.back();
            queue.pop_back();

            bool outgoingFound = false;
            bool incomingFound = false;

            for (auto label : query.labels) {
                if (!toFilters[source][label].empty() && toFilters[source][label].contains(target)) {
                    outgoingFound = true;
                }

                if (!fromFilters[target][label].empty() && fromFilters[target][label].contains(source)) {
                    incomingFound = true;
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

    size_t BloomPathIndex::indexSize() const {
        size_t size = 0;

        for (auto vertex = 0; vertex < getGraph().getVertexCount(); vertex++) {
            for (auto label = 0; label < getGraph().getLabelCount(); label++) {
                size += toFilters[vertex][label].sizeInBytes();
                size += fromFilters[vertex][label].sizeInBytes();
            }
        }

        return size;
    }
}