#include "BloomInFrequentIndex.hpp"

namespace lcr {
    void BloomInFrequentIndex::train() {
        auto &graph = getGraph();

        std::vector<Vertex> order;
        vertexOrderByDegree(graph, order);

        VertexLabelFreqQueue queue;

        toFilters.resize(graph.getVertexCount());

        for (auto i = 0u; i < graph.getVertexCount(); i++) {
            toFilters[i].resize(graph.getLabelCount());
        }

        std::vector<std::vector<LabelSet>> vertexLookup(graph.getVertexCount());

        for (auto i = 0u; i < graph.getVertexCount(); i++) {
            auto vertex = order[i];

            forwardBFS(queue, vertex, vertexLookup);

            for (auto &lookupArray : vertexLookup) {
                lookupArray.clear();
            }
        }
    }

    void BloomInFrequentIndex::forwardBFS(VertexLabelFreqQueue &queue, Vertex origin,
                                          std::vector<std::vector<LabelSet>> &vertexLookup) {
        auto &graph = getGraph();
        queue.emplace(origin, graph.getLabelCount());

        while (!queue.empty()) {
            auto current = queue.top();
            queue.pop();

            if (!tryInsertToVertex(origin, current.vertex, vertexLookup, current.labelSet, current.labelFrequencies)) {
                continue;
            }

            auto it = graph.getConnected(current.vertex);

            while (it.next()) {
                auto &edge = *it;

                if (edge.target == origin) {
                    continue;
                }

                queue.emplace(edge.target, current.labelFrequencies, current.labelSet, current.distance,
                              edge.label);
            }
        }
    }

    bool BloomInFrequentIndex::tryInsertToVertex(Vertex vertex, Vertex target,
                                                 std::vector<std::vector<LabelSet>> &vertexLookup,
                                                 const LabelSet &labelSet,
                                                 const std::vector<uint32_t> &labelFrequencies) {
        if (vertex == target) {
            return true;
        }

        auto &index = vertexLookup[target];

        for (auto &existingLabelSet : index) {
            if (existingLabelSet.is_subset_of(labelSet)) {
                return false;
            }
        }

        auto minLabel = 0u;
        auto minCount = std::numeric_limits<uint32_t>::max();

        for (uint32_t label = 0; label < labelFrequencies.size(); label++) {
            if (minCount > labelFrequencies[label] && labelSet[label]) {
                minCount = labelFrequencies[label];
                minLabel = label;
            }
        }

        if (toFilters[vertex][minLabel].empty()) {
            toFilters[vertex][minLabel].setup(labelBitSize);
        }

        toFilters[vertex][minLabel].add(target);
        index.emplace_back(labelSet);
        return true;
    }

    bool BloomInFrequentIndex::query(const LCRQuery &query) {
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

        while (!queue.empty()) {
            source = queue.back();
            queue.pop_back();

            if (visited[source]) {
                continue;
            }

            bool found = false;

            for (auto label = 0u; label < labelSet.size(); label++) {
                if (labelSet[label] && !toFilters[source][label].empty()) {
                    if (toFilters[source][label].contains(target)) {
                        found = true;
                    }
                }
            }

            if (!found) {
                continue;
            }

            visited[source] = true;

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
                    queue.emplace_back(edge.target);
                }
            }
        }

        return false;
    }

    size_t BloomInFrequentIndex::indexSize() const {
        size_t size = 0;

        for (auto vertex = 0; vertex < getGraph().getVertexCount(); vertex++) {
            for (auto label = 0; label < getGraph().getLabelCount(); label++) {
                size += toFilters[vertex][label].sizeInBytes();
            }
        }

        return size;
    }
}