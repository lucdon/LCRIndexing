#include "PerLabelGraph.hpp"
#include "utility/Format.hpp"

std::ostream &operator <<(std::ostream &out, const PerLabelGraph &graph) {
    out << "G = (V=" << graph.getVertexCount() << ", E=" << graph.getEdgeCount() << ", L=" << graph.getLabelCount()
        << ")" << std::endl;

    auto vertices = double(graph.getVertexCount());
    auto labels = double(graph.getLabelCount());

    double vertexCombinations = vertices * (vertices - 1.0);
    double labelCombinations = (std::pow(2.0, labels)) - 1.0;

    out << "TC size = ";
    formatMemory(out, uint64_t(vertexCombinations * labelCombinations / 8.0));

    return out;
}

void labelDistribution(const PerLabelGraph &perLabelGraph, std::vector<std::pair<uint32_t, Label>> &distribution) {
    distribution.resize(perLabelGraph.getLabelCount());

    for (auto i = 0u; i < perLabelGraph.getLabelCount(); i++) {
        distribution[i].first = perLabelGraph.getEdgeCount(i);
        distribution[i].second = i;
    }

    std::sort(distribution.begin(), distribution.end(), std::greater<>());
}

void orderLabelsByFrequency(const PerLabelGraph &perLabelGraph, std::vector<Label> &order) {
    std::vector<std::pair<uint32_t, Label>> numEdgesByLabel;
    labelDistribution(perLabelGraph, numEdgesByLabel);

    order.clear();
    order.resize(numEdgesByLabel.size());

    for (uint32_t i = 0; i < numEdgesByLabel.size(); i++) {
        order[i] = numEdgesByLabel[i].second;
    }
}

std::ostream &printLabelDistribution(std::ostream &out, const PerLabelGraph &perLabelGraph) {
    uint32_t totalEdges = perLabelGraph.getEdgeCount();

    std::vector<std::pair<uint32_t, Label>> numEdgesByLabel;
    labelDistribution(perLabelGraph, numEdgesByLabel);

    out << "Label Distribution over, " << perLabelGraph.getLabelCount() << " labels and "
        << perLabelGraph.getEdgeCount() << " edges" << std::endl;

    uint32_t cumEdges = 0;

    for (auto i = 0u; i < 25 && i < perLabelGraph.getLabelCount(); i++) {
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