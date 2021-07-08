#include "BloomGraphIndex.hpp"

namespace lcr {
    void BloomGraphIndex::train() {
        auto &graph = getGraph();

        std::vector<Vertex> order;
        vertexOrderByDegree(graph, order);

        VertexQueue queue;
        bloomFilters.resize(graph.getVertexCount());

        std::vector<std::vector<LabelSet>> vertexLookup(graph.getVertexCount());

        for (auto i = 0u; i < getGraph().getVertexCount(); i++) {
            auto vertex = order[i];

            bloomFilters[vertex].setup(labelBitSize);

            createIndexForVertex(queue, vertex, vertexLookup);

            for (auto &lookupArray : vertexLookup) {
                lookupArray.clear();
            }
        }
    }

    void BloomGraphIndex::createIndexForVertex(VertexQueue &queue, Vertex landmark,
                                               std::vector<std::vector<LabelSet>> &vertexLookup) {
        auto &graph = getGraph();

        queue.emplace(landmark, graph.getLabelCount(), -1);
        VertexEntry current(landmark, graph.getLabelCount(), -1);

        while (!queue.empty()) {
            current = queue.top();
            queue.pop();

            if (!tryInsert(landmark, current.vertex, current.labelSet, vertexLookup)) {
                continue;
            }

            auto it = graph.getConnected(current.vertex);

            while (it.next()) {
                auto &edge = *it;

                if (edge.target == landmark) {
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

    bool BloomGraphIndex::tryInsert(Vertex vertex, Vertex target, const LabelSet &labelSet,
                                    std::vector<std::vector<LabelSet>> &vertexLookup) {
        if (target == vertex) {
            return true;
        }

        auto &index = vertexLookup[target];

        for (auto &existingLabelSet : index) {
            if (existingLabelSet.is_subset_of(labelSet)) {
                return false;
            }
        }

        index.emplace_back(labelSet);
        bloomFilters[vertex].addWithSuperSets(target, labelSet);
        return true;
    }

    bool BloomGraphIndex::query(const LCRQuery &query) {
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

        auto hash = boost::hash_value(target);
        boost::hash_combine(hash, boost::hash_value(labelSet));

        while (!queue.empty()) {
            source = queue.front();
            queue.pop_front();

            if (!bloomFilters[source].containsHash(hash)) {
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

    size_t BloomGraphIndex::indexSize() const {
        size_t size = 0;

        for (auto i = 0; i < getGraph().getVertexCount(); i++) {
            size += bloomFilters[i].sizeInBytes();
        }

        return size;
    }
}