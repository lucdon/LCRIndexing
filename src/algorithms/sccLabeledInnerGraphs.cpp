#include "graphs/LabeledGraph.hpp"
#include "graphs/SCCGraph.hpp"

void sccLabeledInnerGraphs(const PerLabelGraph &labeledGraph, const SCCGraph &sccGraph,
                           std::vector<std::unique_ptr<PerLabelGraph>> &innerGraphsOut) {
    innerGraphsOut.resize(sccGraph.getComponentCount());

    for (auto source = 0; source < labeledGraph.getVertexCount(); source++) {
        auto sourceComponent = sccGraph.getComponentIndex(source);
        auto &innerGraph = innerGraphsOut[sourceComponent];

        if (innerGraph == nullptr) {
            innerGraph = std::make_unique<PerLabelGraph>();
        }

        for (auto label = 0u; label < labeledGraph.getLabelCount(); label++) {
            for (auto target : labeledGraph.getConnected(source, label)) {
                auto targetComponent = sccGraph.getComponentIndex(target);

                if (sourceComponent != targetComponent) {
                    continue;
                }

                if (innerGraph->getVertexCount() == 0) {
                    innerGraph->setSizes(labeledGraph.getVertexCount(), labeledGraph.getLabelCount());
                }

                innerGraph->addEdge(source, target, label);
            }
        }
    }
}

void sccLabeledInnerGraphs(const LabeledGraph &labeledGraph, const SCCGraph &sccGraph,
                           std::vector<std::unique_ptr<LabeledGraph>> &innerGraphsOut) {
    innerGraphsOut.resize(sccGraph.getComponentCount());

    for (auto source = 0; source < labeledGraph.getVertexCount(); source++) {
        auto sourceComponent = sccGraph.getComponentIndex(source);
        auto &innerGraph = innerGraphsOut[sourceComponent];

        if (innerGraph == nullptr) {
            innerGraph = std::make_unique<LabeledGraph>();
        }

        for (auto &vertexAndLabels : labeledGraph.getConnected(source)) {
            auto targetComponent = sccGraph.getComponentIndex(vertexAndLabels.first);

            if (sourceComponent != targetComponent) {
                continue;
            }

            if (innerGraph->getVertexCount() == 0) {
                innerGraph->setSizes(labeledGraph.getVertexCount(), labeledGraph.getLabelCount());
            }

            innerGraph->addEdgeNoChecks(source, vertexAndLabels.first, vertexAndLabels.second);
        }
    }
}

void sccLabeledInnerGraphs(const LabeledEdgeGraph &labeledGraph, const SCCGraph &sccGraph,
                           std::vector<std::unique_ptr<LabeledEdgeGraph>> &innerGraphsOut) {
    innerGraphsOut.resize(sccGraph.getComponentCount());

    for (auto source = 0; source < labeledGraph.getVertexCount(); source++) {
        auto sourceComponent = sccGraph.getComponentIndex(source);
        auto &innerGraph = innerGraphsOut[sourceComponent];

        if (innerGraph == nullptr) {
            innerGraph = std::make_unique<LabeledEdgeGraph>();
        }

        auto it = labeledGraph.getConnected(source);

        while (it.next()) {
            auto &edge = *it;

            auto targetComponent = sccGraph.getComponentIndex(edge.target);

            if (sourceComponent != targetComponent) {
                continue;
            }

            if (innerGraph->getVertexCount() == 0) {
                innerGraph->setSizes(labeledGraph.getVertexCount(), labeledGraph.getLabelCount(), 0);
            }

            innerGraph->addEdge(source, edge.target, edge.label);
        }
    }

    for (auto &innerGraph : innerGraphsOut) {
        if (innerGraph->getVertexCount() != 0) {
            innerGraph->optimize();
        }
    }
}