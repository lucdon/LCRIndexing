#include "HybridBFSIndex.hpp"

static std::vector<uint32_t> frontiers;

static uint32_t baseStep = 0;
static uint32_t previousStep = 0;
static uint32_t currentStep = 0;

void HybridBFSIndex::train() {
    frontiers.resize(getGraph().getVertexCount());
}

bool HybridBFSIndex::query(const ReachQuery &query) {
    auto &graph = getGraph();
    auto& sccGraph = getSCCGraph();

    auto source = sccGraph.getComponentIndex(query.source);
    auto target = sccGraph.getComponentIndex(query.target);

    if (source == target) {
        return true;
    }

    // Base case, since we are querying the strongly connected graph which is a topological sorted DAG.
    if (source < target) {
        return false;
    }

    uint32_t alpha = 14;
    uint32_t beta = 24;

    uint32_t nodesInFrontier = 1;
    uint32_t totalNodes = getGraph().getVertexCount();
    uint32_t edgesInFrontier = graph.getConnected(source).size();
    uint32_t unexploredEdges = graph.getEdgeCount() - edgesInFrontier;

    previousStep++;
    currentStep++;

    baseStep = currentStep - 1;
    frontiers[source] = currentStep;

    bool shouldDoBottomUp = false;

    while (nodesInFrontier != 0) {
        currentStep++;
        nodesInFrontier = 0;
        edgesInFrontier = 0;

        if (shouldDoBottomUp) {
            if (bottomUp(graph, target, nodesInFrontier, edgesInFrontier)) {
                previousStep++;
                return true;
            }

            unexploredEdges -= edgesInFrontier;

            // Now we calculate when we should switch to top down.
            if (nodesInFrontier < totalNodes / beta) {
                shouldDoBottomUp = false;
            }
        } else {
            if (topDown(graph, target, nodesInFrontier, edgesInFrontier)) {
                previousStep++;
                return true;
            }

            unexploredEdges -= edgesInFrontier;

            // Now we calculate when we should switch to bottom up.
            if (edgesInFrontier > unexploredEdges / alpha) {
                shouldDoBottomUp = true;
            }
        }

        previousStep++;
    }

    return false;
}

bool HybridBFSIndex::bottomUp(const DiGraph &graph, Vertex target, uint32_t &nodesInFrontier, uint32_t &edgesInFrontier) {
    for (auto vertex = 0; vertex < graph.getVertexCount(); vertex++) {
        if (frontiers[vertex] > baseStep) {
            continue;
        }

        for (auto &previous : graph.getReverseConnected(vertex)) {
            if (frontiers[previous] != previousStep) {
                continue;
            }

            if (vertex == target) {
                return true;
            }

            frontiers[vertex] = currentStep;
            nodesInFrontier++;
            edgesInFrontier += graph.getConnected(vertex).size();
            break;
        }
    }

    return false;
}

bool HybridBFSIndex::topDown(const DiGraph &graph, Vertex target, uint32_t &nodesInFrontier, uint32_t &edgesInFrontier) {
    for (auto vertex = 0; vertex < graph.getVertexCount(); vertex++) {
        if (frontiers[vertex] != previousStep) {
            continue;
        }

        for (auto &next : graph.getConnected(vertex)) {
            if (frontiers[next] > baseStep) {
                continue;
            }

            if (next == target) {
                return true;
            }

            frontiers[next] = currentStep;
            nodesInFrontier++;
            edgesInFrontier += graph.getConnected(next).size();
        }
    }

    return false;
}
