#include "graphs/LabeledGraph.hpp"

std::unique_ptr<LabeledGraph> compressLabeledGraph(const PerLabelGraph& perLabelGraph) {
    auto labeledGraphPtr = std::make_unique<LabeledGraph>();
    auto& labeledGraph = *labeledGraphPtr;

    labeledGraph.setSizes(perLabelGraph.getVertexCount(), perLabelGraph.getLabelCount());

    auto labelCount = perLabelGraph.getLabelCount();
    auto edgeCount = perLabelGraph.getEdgeCount();
    auto labeledEdgeCount = 0u;

    std::vector<Vertex> hasEdge(perLabelGraph.getVertexCount(), -1);
    std::vector<Vertex> edges;
    std::vector<LabelSet> labelPerEdge(perLabelGraph.getVertexCount());

    for (auto &labelSet : labelPerEdge) {
        labelSet.resize(labelCount);
    }

    edges.reserve(perLabelGraph.getVertexCount());

    // First construct fromAdj
    for (auto source = 0u; source < perLabelGraph.getVertexCount(); source++) {
        for (auto label = 0u; label < perLabelGraph.getLabelCount(); label++) {
            for (auto target : perLabelGraph.getConnected(source, label)) {
                if (hasEdge[target] != (int) source) {
                    hasEdge[target] = (int) source;
                    labeledEdgeCount++;
                    edges.emplace_back(target);
                }

                if (!labelPerEdge[target][label]) {
                    labelPerEdge[target][label] = true;
                }
            }
        }

        labeledGraph.setEdgesNoChecks(source, edges, labelPerEdge);
        edges.clear();
    }

    std::fill(hasEdge.begin(), hasEdge.end(), -1);

    // Construct toAdj
    for (auto target = 0u; target < perLabelGraph.getVertexCount(); target++) {
        for (auto label = 0u; label < perLabelGraph.getLabelCount(); label++) {
            for (auto source : perLabelGraph.getReverseConnected(target, label)) {
                if (hasEdge[source] != (int) target) {
                    hasEdge[source] = (int) target;
                    edges.emplace_back(source);
                }

                if (!labelPerEdge[source][label]) {
                    labelPerEdge[source][label] = true;
                }
            }
        }

        labeledGraph.setRevEdgesNoChecks(target, edges, labelPerEdge);
        edges.clear();
    }

    labeledGraph.setEdgeSizes(edgeCount, labeledEdgeCount);
    return labeledGraphPtr;
}