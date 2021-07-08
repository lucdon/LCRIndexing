#include "graphs/LabeledGraph.hpp"

std::unique_ptr<DiGraph>
mergeGraphForLabels(const PerLabelGraph &labeledGraph, const LabelSet &labelSet, MergedGraphStats &outStats) {
    auto graph = std::make_unique<DiGraph>();
    graph->setVertices(labeledGraph.getVertexCount());

    boost::dynamic_bitset<> visited(labeledGraph.getVertexCount());
    std::vector<uint32_t> edges;

    edges.reserve(labeledGraph.getVertexCount());

    std::vector<uint32_t> labels;
    size_t maxEdgeCount = 0;

    for (auto label = 0u; label < labeledGraph.getLabelCount(); label++) {
        if (!labelSet[label]) {
            continue;
        }

        labels.emplace_back(label);
        maxEdgeCount = std::max<size_t>(maxEdgeCount, labeledGraph.getEdgeCount(label));
    }

    if (labels.empty()) {
        return graph;
    }

    for (auto source = 0u; source < labeledGraph.getVertexCount(); source++) {
        for (auto label : labels) {
            for (auto target : labeledGraph.getConnected(source, label)) {
                if (visited[target]) {
                    continue;
                }

                visited[target] = true;
                edges.emplace_back(target);
            }
        }

        graph->addEdgesNoChecks(source, edges);
        edges.clear();
        visited.reset();
    }

    if (labels.size() > 1) {
        outStats.increase = (int64_t(maxEdgeCount) - int64_t(graph->getEdgeCount()));
        outStats.increasePercentage = double(outStats.increase) / double(maxEdgeCount) * 100.0;
    } else {
        // Base case for which we just copy a single label graph.
        outStats.increase = int64_t(maxEdgeCount);
        outStats.increasePercentage = 100;
    }

    return graph;
}

std::unique_ptr<DiGraph>
mergeGraphForLabels(const LabeledGraph &labeledGraph, const LabelSet &labelSet, MergedGraphStats &outStats) {
    auto graph = std::make_unique<DiGraph>();
    graph->setVertices(labeledGraph.getVertexCount());

    boost::dynamic_bitset<> visited(labeledGraph.getVertexCount());
    std::vector<uint32_t> edges;

    edges.reserve(labeledGraph.getVertexCount());
    size_t maxEdgeCount = 0;

    for (auto label = 0u; label < labeledGraph.getLabelCount(); label++) {
        if (!labelSet[label]) {
            continue;
        }

        maxEdgeCount = std::max(maxEdgeCount, labeledGraph.getEdgeCount(label));
    }

    if (labelSet.count() == 0) {
        return graph;
    }

    for (auto source = 0u; source < labeledGraph.getVertexCount(); source++) {
        for (auto& vertexAndLabelSet : labeledGraph.getConnected(source)) {
            if (!vertexAndLabelSet.second.intersects(labelSet)) {
                continue;
            }

            if (visited[vertexAndLabelSet.first]) {
                continue;
            }

            visited[vertexAndLabelSet.first] = true;
            edges.emplace_back(vertexAndLabelSet.first);
        }

        graph->addEdgesNoChecks(source, edges);
        edges.clear();
        visited.reset();
    }

    if (labelSet.count() > 1) {
        outStats.increase = (int64_t(maxEdgeCount) - int64_t(graph->getEdgeCount()));
        outStats.increasePercentage = double(outStats.increase) / double(maxEdgeCount) * 100.0;
    } else {
        // Base case for which we just copy a single label graph.
        outStats.increase = int64_t(maxEdgeCount);
        outStats.increasePercentage = 100;
    }

    return graph;
}

std::unique_ptr<DiGraph>
mergeGraphForLabels(const LabeledEdgeGraph &labeledGraph, const LabelSet &labelSet, MergedGraphStats &outStats) {
    auto graph = std::make_unique<DiGraph>();
    graph->setVertices(labeledGraph.getVertexCount());

    boost::dynamic_bitset<> visited(labeledGraph.getVertexCount());
    std::vector<uint32_t> edges;

    edges.reserve(labeledGraph.getVertexCount());
    size_t maxEdgeCount = 0;

    for (auto label = 0u; label < labeledGraph.getLabelCount(); label++) {
        if (!labelSet[label]) {
            continue;
        }

        maxEdgeCount = std::max(maxEdgeCount, labeledGraph.getEdgeCount(label));
    }

    if (labelSet.count() == 0) {
        return graph;
    }

    for (auto source = 0u; source < labeledGraph.getVertexCount(); source++) {
        auto it = labeledGraph.getConnected(source, labelSet);

        while(it.next()) {
            auto& edge = *it;

            if (visited[edge.target]) {
                continue;
            }

            visited[edge.target] = true;
            edges.emplace_back(edge.target);
        }

        graph->addEdgesNoChecks(source, edges);
        edges.clear();
        visited.reset();
    }

    if (labelSet.count() > 1) {
        outStats.increase = (int64_t(maxEdgeCount) - int64_t(graph->getEdgeCount()));
        outStats.increasePercentage = double(outStats.increase) / double(maxEdgeCount) * 100.0;
    } else {
        // Base case for which we just copy a single label graph.
        outStats.increase = int64_t(maxEdgeCount);
        outStats.increasePercentage = 100;
    }

    return graph;
}
