#include "graphs/PerLabelGraph.hpp"
#include "graphs/LabeledGraph.hpp"

std::unique_ptr<PerLabelGraph>
createVirtualLabelGraph(const PerLabelGraph &labeledGraph, const std::vector<Label> &labelOrder,
                        uint32_t numMostFrequent, uint32_t numVirtual, std::vector<Label> &outVirtualLabelMapping) {
    outVirtualLabelMapping.resize(labeledGraph.getLabelCount());

    uint32_t virtualLabelCounter = 0;
    std::vector<std::vector<Label>> virtualLabelToLabels(numMostFrequent + numVirtual);

    for (auto i = 0u; i < numMostFrequent; i++) {
        Label mostFrequent = labelOrder[i];
        outVirtualLabelMapping[mostFrequent] = virtualLabelCounter;
        virtualLabelToLabels[virtualLabelCounter].emplace_back(mostFrequent);
        virtualLabelCounter++;
    }

    for (auto i = numMostFrequent; i < labeledGraph.getLabelCount(); i++) {
        auto virtualLabelIndex = i % numVirtual;
        Label mostFrequent = labelOrder[i];

        outVirtualLabelMapping[mostFrequent] = virtualLabelCounter + virtualLabelIndex;
        virtualLabelToLabels[virtualLabelCounter + virtualLabelIndex].emplace_back(mostFrequent);
    }

    auto graph = std::make_unique<PerLabelGraph>();
    graph->setSizes(labeledGraph.getVertexCount(), numMostFrequent + numVirtual);

    boost::dynamic_bitset<> visited(graph->getVertexCount());
    std::vector<Vertex> edges(graph->getVertexCount());

    for (auto virtualLabel = 0u; virtualLabel < numMostFrequent + numVirtual; virtualLabel++) {
        auto &labels = virtualLabelToLabels[virtualLabel];

        if (labels.empty()) {
            continue;
        }

        if (labels.size() == 1) {
            for (auto vertex = 0u; vertex < graph->getVertexCount(); vertex++) {
                graph->addEdgesNoChecks(vertex, virtualLabel, labeledGraph.getConnected(vertex, labels[0]));
            }
        } else {
            for (auto vertex = 0u; vertex < graph->getVertexCount(); vertex++) {
                visited.reset();
                edges.clear();

                for (auto label : labels) {
                    for (auto connected : labeledGraph.getConnected(vertex, label)) {
                        if (!visited[connected]) {
                            visited[connected] = true;
                            edges.emplace_back(connected);
                        }
                    }
                }

                graph->addEdgesNoChecks(vertex, virtualLabel, edges);
            }
        }
    }

    return graph;
}

std::unique_ptr<LabeledGraph>
createVirtualLabelGraph(const LabeledGraph &labeledGraph, const std::vector<Label> &labelOrder,
                        uint32_t numMostFrequent, uint32_t numVirtual, std::vector<Label> &outVirtualLabelMapping) {
    outVirtualLabelMapping.resize(labeledGraph.getLabelCount());
    uint32_t virtualLabelCounter = 0;

    for (auto i = 0; i < numMostFrequent; i++) {
        Label mostFrequent = labelOrder[i];
        outVirtualLabelMapping[mostFrequent] = virtualLabelCounter;
        virtualLabelCounter++;
    }

    for (auto i = numMostFrequent; i < labeledGraph.getLabelCount(); i++) {
        auto virtualLabelIndex = i % numVirtual;
        Label mostFrequent = labelOrder[i];

        outVirtualLabelMapping[mostFrequent] = virtualLabelCounter + virtualLabelIndex;
    }

    auto graph = std::make_unique<LabeledGraph>();
    graph->setSizes(labeledGraph.getVertexCount(), numMostFrequent + numVirtual);

    uint32_t edgeCount = 0;
    uint32_t labeledEdgeCount = 0;

    for (auto vertex = 0u; vertex < graph->getVertexCount(); vertex++) {
        for (auto &vertexAndLabelSet : labeledGraph.getConnected(vertex)) {
            LabelSet virtualLabels(numMostFrequent + numVirtual);

            for (auto label = 0u; label < labeledGraph.getLabelCount(); label++) {
                if (vertexAndLabelSet.second[label]) {
                    virtualLabels[outVirtualLabelMapping[label]] = true;
                }
            }

            uint32_t count = uint32_t(virtualLabels.count());
            labeledEdgeCount += count;

            if (count != 0) {
                edgeCount += 1;
                graph->addEdgeNoChecks(vertex, vertexAndLabelSet.first, virtualLabels);
            }
        }
    }

    graph->setEdgeSizes(edgeCount, labeledEdgeCount);
    return graph;
}

std::unique_ptr<LabeledEdgeGraph>
createVirtualLabelGraph(const LabeledEdgeGraph &labeledGraph, const std::vector<Label> &labelOrder,
                        uint32_t numMostFrequent, uint32_t numVirtual, std::vector<Label> &outVirtualLabelMapping) {
    outVirtualLabelMapping.resize(labeledGraph.getLabelCount());
    uint32_t virtualLabelCounter = 0;

    for (auto i = 0; i < numMostFrequent; i++) {
        Label mostFrequent = labelOrder[i];
        outVirtualLabelMapping[mostFrequent] = virtualLabelCounter;
        virtualLabelCounter++;
    }

    for (auto i = numMostFrequent; i < labeledGraph.getLabelCount(); i++) {
        auto virtualLabelIndex = i % numVirtual;
        Label mostFrequent = labelOrder[i];

        outVirtualLabelMapping[mostFrequent] = virtualLabelCounter + virtualLabelIndex;
    }

    auto graph = std::make_unique<LabeledEdgeGraph>();
    graph->setSizes(labeledGraph.getVertexCount(), numMostFrequent + numVirtual, labeledGraph.getEdgeCount());

    for (auto vertex = 0u; vertex < graph->getVertexCount(); vertex++) {
        auto it = labeledGraph.getConnected(vertex);

        while (it.next()) {
            auto &edge = *it;
            auto label = outVirtualLabelMapping[edge.label];
            graph->addEdge(edge.source, edge.target, label);
        }
    }

    graph->optimize();
    return graph;
}