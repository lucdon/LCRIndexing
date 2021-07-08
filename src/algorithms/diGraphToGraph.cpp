#include "graphs/Graph.hpp"
#include "graphs/DiGraph.hpp"

std::unique_ptr<Graph> diGraphToGraph(const DiGraph& diGraph) {
    auto graphPtr = std::make_unique<Graph>();
    auto& graph = *graphPtr;

    graph.setVertices(graph.getVertexCount());

    std::vector<Vertex> verticesToAdd;
    boost::dynamic_bitset<> visitedVertices;

    visitedVertices.resize(diGraph.getVertexCount());
    verticesToAdd.reserve(diGraph.getVertexCount());

    for (auto vertex = 0u; vertex < diGraph.getVertexCount(); vertex++) {
        for (auto target : diGraph.getConnected(vertex)) {
            if (visitedVertices[target]) {
                continue;
            }

            visitedVertices[target] = true;
            verticesToAdd.emplace_back(target);
        }

        for (auto source : diGraph.getReverseConnected(vertex)) {
            if (visitedVertices[source]) {
                continue;
            }

            visitedVertices[source] = true;
            verticesToAdd.emplace_back(source);
        }

        std::sort(verticesToAdd.begin(), verticesToAdd.end());
        graph.addEdgesNoChecks(vertex, verticesToAdd);

        // Reset state.
        for (auto added : verticesToAdd) {
            visitedVertices[added] = false;
        }

        verticesToAdd.clear();
    }

    return graphPtr;
}
