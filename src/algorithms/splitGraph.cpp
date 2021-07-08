#include "graphs/LabeledGraph.hpp"

std::unique_ptr<LabeledEdgeGraph>
splitGraph(const LabeledEdgeGraph &labeledGraph, const LabelSet &labels, const std::vector<Label> &labelMapping) {
    auto graph = std::make_unique<LabeledEdgeGraph>();
    graph->setSizes(labeledGraph.getVertexCount(), labels.count(), 0);

    for (auto vertex = 0u; vertex < labeledGraph.getVertexCount(); vertex++) {
        auto it = labeledGraph.getConnected(vertex);

        while (it.next()) {
            auto &edge = *it;

            if (!labels[edge.label]) {
                continue;
            }

            graph->addEdge(edge.source, edge.target, labelMapping[edge.label]);
        }
    }

    graph->optimize();
    return graph;
}