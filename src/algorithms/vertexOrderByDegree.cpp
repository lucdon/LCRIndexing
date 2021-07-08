#include "graphs/SCCGraph.hpp"
#include "graphs/LabeledGraph.hpp"

static inline size_t calculateDegree(const DiGraph &graph, Vertex vertex) {
    return (graph.getConnected(vertex).size() + 1) * (graph.getReverseConnected(vertex).size() + 1);
}

static inline size_t calculateDegree(const LabeledGraph &graph, Vertex vertex) {
    // Does not include label count.
    return (graph.getConnected(vertex).size() + 1) * (graph.getReverseConnected(vertex).size() + 1);
}

static inline size_t calculateDegree(const LabeledEdgeGraph &graph, Vertex vertex) {
    auto totalIncoming = 1u;
    auto totalOutgoing = 1u;

    auto it = graph.getConnected(vertex);

    while (it.next()) {
        totalOutgoing++;
    }

    auto revIt = graph.getReverseConnected(vertex);

    while (revIt.next()) {
        totalIncoming++;
    }

    return totalIncoming * totalOutgoing;
}

static inline size_t calculateDegree(const PerLabelGraph &graph, Vertex vertex) {
    auto totalIncoming = 1u;
    auto totalOutgoing = 1u;

    for (auto label = 0u; label < graph.getLabelCount(); label++) {
        totalOutgoing += uint32_t(graph.getConnected(vertex, label).size());
        totalIncoming += uint32_t(graph.getReverseConnected(vertex, label).size());
    }

    return totalIncoming * totalOutgoing;
}

void vertexOrderByDegree(const DiGraph &graph, std::vector<Vertex> &order) {
    std::vector<std::pair<size_t, Vertex>> degreeAndVertexPairs(graph.getVertexCount());

    order.resize(graph.getVertexCount());

    for (Vertex i = 0; i < graph.getVertexCount(); i++) {
        degreeAndVertexPairs[i] = std::make_pair(calculateDegree(graph, i), i);
    }

    std::sort(degreeAndVertexPairs.begin(), degreeAndVertexPairs.end(), std::greater<>());

    for (Vertex i = 0; i < graph.getVertexCount(); i++) {
        order[i] = degreeAndVertexPairs[i].second;
    }
}

void vertexOrderByDegree(const LabeledEdgeGraph &graph, std::vector<Vertex> &order) {
    std::vector<std::pair<size_t, Vertex>> degreeAndVertexPairs(graph.getVertexCount());

    order.resize(graph.getVertexCount());

    for (Vertex i = 0; i < graph.getVertexCount(); i++) {
        degreeAndVertexPairs[i] = std::make_pair(calculateDegree(graph, i), i);
    }

    std::sort(degreeAndVertexPairs.begin(), degreeAndVertexPairs.end(), std::greater<>());

    for (Vertex i = 0; i < graph.getVertexCount(); i++) {
        order[i] = degreeAndVertexPairs[i].second;
    }
}

void vertexOrderByDegree(const LabeledGraph &graph, std::vector<Vertex> &order) {
    std::vector<std::pair<size_t, Vertex>> degreeAndVertexPairs(graph.getVertexCount());

    order.resize(graph.getVertexCount());

    for (Vertex i = 0; i < graph.getVertexCount(); i++) {
        degreeAndVertexPairs[i] = std::make_pair(calculateDegree(graph, i), i);
    }

    std::sort(degreeAndVertexPairs.begin(), degreeAndVertexPairs.end(), std::greater<>());

    for (Vertex i = 0; i < graph.getVertexCount(); i++) {
        order[i] = degreeAndVertexPairs[i].second;
    }
}

void vertexOrderByDegree(const PerLabelGraph &graph, std::vector<Vertex> &order) {
    std::vector<std::pair<size_t, Vertex>> degreeAndVertexPairs(graph.getVertexCount());

    order.resize(graph.getVertexCount());

    for (Vertex i = 0; i < graph.getVertexCount(); i++) {
        degreeAndVertexPairs[i] = std::make_pair(calculateDegree(graph, i), i);
    }

    std::sort(degreeAndVertexPairs.begin(), degreeAndVertexPairs.end(), std::greater<>());

    for (Vertex i = 0; i < graph.getVertexCount(); i++) {
        order[i] = degreeAndVertexPairs[i].second;
    }
}