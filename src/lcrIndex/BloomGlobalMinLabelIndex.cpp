#include "BloomGlobalMinLabelIndex.hpp"

namespace lcr {
    void BloomGlobalMinLabelIndex::train() {
        auto &graph = getGraph();

        std::vector<uint32_t> labelFrequencies(graph.getLabelCount());

        for (auto vertex = 0u; vertex < graph.getVertexCount(); vertex++) {
            auto it = graph.getConnected(vertex);

            while (it.next()) {
                auto &edge = *it;

                labelFrequencies[edge.label]++;
            }
        }

        std::vector<Vertex> order;
        vertexOrderByDegree(graph, order);

        VertexQueue queue;

        toFilters.resize(graph.getVertexCount());

        for (auto i = 0u; i < graph.getVertexCount(); i++) {
            toFilters[i].resize(graph.getLabelCount());
        }

        std::vector<std::vector<LabelSet>> vertexLookup(graph.getVertexCount());

        for (auto i = 0u; i < graph.getVertexCount(); i++) {
            auto vertex = order[i];

            forwardBFS(queue, vertex, vertexLookup, labelFrequencies);

            for (auto &lookupArray : vertexLookup) {
                lookupArray.clear();
            }
        }
    }

    void BloomGlobalMinLabelIndex::forwardBFS(VertexQueue &queue, Vertex origin,
                                              std::vector<std::vector<LabelSet>> &vertexLookup,
                                              const std::vector<uint32_t> &labelFrequencies) {
        auto &graph = getGraph();
        queue.emplace(origin, graph.getLabelCount(), -1);

        while (!queue.empty()) {
            auto current = queue.top();
            queue.pop();

            if (!tryInsertToVertex(origin, current.vertex, vertexLookup, current.labelSet, labelFrequencies)) {
                continue;
            }

            auto it = graph.getConnected(current.vertex);

            while (it.next()) {
                auto &edge = *it;

                if (edge.target == origin) {
                    continue;
                }

                LabelSet labelSet(graph.getLabelCount());

                labelSet |= current.labelSet;
                labelSet[edge.label] = true;

                // Distance is equal to the number of labels
                auto distance = (int) labelSet.count();
                queue.emplace(edge.target, labelSet, distance);
            }
        }
    }

    bool BloomGlobalMinLabelIndex::tryInsertToVertex(Vertex vertex, Vertex target,
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

    bool BloomGlobalMinLabelIndex::query(const LCRQuery &query) {
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
                        break;
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

    size_t BloomGlobalMinLabelIndex::indexSize() const {
        size_t size = 0;

        for (auto vertex = 0; vertex < getGraph().getVertexCount(); vertex++) {
            for (auto label = 0; label < getGraph().getLabelCount(); label++) {
                size += toFilters[vertex][label].sizeInBytes();
            }
        }

        return size;
    }
}