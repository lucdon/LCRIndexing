#include "graphs/SCCGraph.hpp"

bool reachabilityBFS(const DiGraph &graph, Vertex source, Vertex target) {
    if (source == target) {
        return true;
    }

    boost::dynamic_bitset<> visited(graph.getVertexCount());

    std::deque<Vertex> queue;

    visited[source] = true;
    queue.emplace_back(source);

    while (!queue.empty()) {
        source = queue.front();
        queue.pop_front();

        for (auto vertex : graph.getConnected(source)) {
            if (vertex == target) {
                return true;
            }

            if (!visited[vertex]) {
                visited[vertex] = true;
                queue.emplace_back(vertex);
            }
        }
    }

    return false;
}