#include "LabeledGraph.hpp"
#include "utility/Format.hpp"

std::ostream &operator <<(std::ostream &out, const LabeledGraph &graph) {
    out << "G = (V=" << graph.getVertexCount() << ", E=" << graph.getEdgeCount() << ", E_L="
        << graph.getLabeledEdgeCount() << ", L=" << graph.getLabelCount() << ")" << std::endl;

    auto vertices = double(graph.getVertexCount());
    auto labels = double(graph.getLabelCount());

    double vertexCombinations = vertices * (vertices - 1.0);
    double labelCombinations = (std::pow(2.0, labels)) - 1.0;

    out << "TC size = ";
    formatMemory(out, uint64_t(vertexCombinations * labelCombinations / 8.0));

    return out;
}
void labelDistribution(const LabeledGraph &labeledGraph, std::vector<std::pair<uint32_t, Label>> &distribution) {
    distribution.resize(labeledGraph.getLabelCount());

    for (auto i = 0u; i < labeledGraph.getLabelCount(); i++) {
        distribution[i].first = labeledGraph.getEdgeCount(i);
        distribution[i].second = i;
    }

    std::sort(distribution.begin(), distribution.end(), std::greater<>());
}

void orderLabelsByFrequency(const LabeledGraph &labeledGraph, std::vector<Label> &order) {
    std::vector<std::pair<uint32_t, Label>> numEdgesByLabel;
    labelDistribution(labeledGraph, numEdgesByLabel);

    order.clear();
    order.resize(numEdgesByLabel.size());

    for (uint32_t i = 0; i < numEdgesByLabel.size(); i++) {
        order[i] = numEdgesByLabel[i].second;
    }
}

std::ostream &printLabelDistribution(std::ostream &out, const LabeledGraph &labeledGraph) {
    uint32_t totalEdges = labeledGraph.getEdgeCount();

    std::vector<std::pair<uint32_t, Label>> numEdgesByLabel;
    labelDistribution(labeledGraph, numEdgesByLabel);

    out << "Label Distribution over, " << labeledGraph.getLabelCount() << " labels and "
        << labeledGraph.getEdgeCount() << " edges" << std::endl;

    uint32_t cumEdges = 0;

    for (auto i = 0u; i < 25 && i < labeledGraph.getLabelCount(); i++) {
        auto &labelPair = numEdgesByLabel[i];

        out << "Index: ";
        formatWidth(out, i, 6);

        out << "Label: ";
        formatWidth(out, labelPair.second, 6);

        out << "Percentage: " << (double(labelPair.first) / double(totalEdges)) * 100 << "%" << std::endl;

        cumEdges += labelPair.first;
    }

    out << "Rest:                     Percentage: " << (double(totalEdges - cumEdges) / double(totalEdges)) * 100 << "%"
        << std::endl;
    out << std::endl;
    return out;
}

void vertexDistribution(const LabeledGraph &graph, std::vector<std::pair<uint32_t, Vertex>> &order) {
    order.resize(graph.getVertexCount());

    for (Vertex i = 0; i < graph.getVertexCount(); i++) {
        auto degree = graph.getConnected(i).size() + graph.getReverseConnected(i).size();
        order[i] = std::make_pair(uint32_t(degree), i);
    }

    std::sort(order.begin(), order.end(), std::greater<>());
}

std::ostream &printVertexDistribution(std::ostream &out, const LabeledGraph &labeledGraph) {
    uint32_t totalEdges = labeledGraph.getLabeledEdgeCount();

    std::vector<std::pair<uint32_t, Label>> numEdgesByVertex;
    vertexDistribution(labeledGraph, numEdgesByVertex);

    out << "Vertex Distribution over, " << labeledGraph.getVertexCount() << " vertices and "
        << labeledGraph.getLabeledEdgeCount() << " edges" << std::endl;

    uint32_t cumEdges = 0;

    for (auto i = 0u; i < 25 && i < labeledGraph.getVertexCount(); i++) {
        auto &vertexPair = numEdgesByVertex[i];

        out << "Index: ";
        formatWidth(out, i, 6);

        out << "Vertex: ";
        formatWidth(out, vertexPair.second, 6);

        out << "Percentage: " << (double(vertexPair.first) / double(totalEdges)) * 100 << "%" << std::endl;

        cumEdges += vertexPair.first;
    }

    out << "Rest:                     Percentage: " << (double(totalEdges - cumEdges) / double(totalEdges)) * 100 << "%"
        << std::endl;
    out << std::endl;
    return out;
}