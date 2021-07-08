#include "graphs/SCCGraph.hpp"

bool reachabilityDFS(const DiGraph &graph, Vertex source, Vertex target) {
    if (source == target) {
        return true;
    }

    boost::dynamic_bitset<> visited(graph.getVertexCount());

    std::deque<Vertex> stack;

    visited[source] = true;
    stack.emplace_back(source);

    while (!stack.empty()) {
        source = stack.back();
        stack.pop_back();

        for (auto vertex : graph.getConnected(source)) {
            if (vertex == target) {
                return true;
            }

            if (!visited[vertex]) {
                visited[vertex] = true;
                stack.emplace_back(vertex);
            }
        }
    }

    return false;
}