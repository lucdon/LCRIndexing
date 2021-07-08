#include "graphs/SCCGraph.hpp"

void singleSourceReachable(const DiGraph &graph, Vertex source, std::vector <Vertex> &outReachableSet) {
    boost::dynamic_bitset<> visited(graph.getVertexCount());

    std::deque<Vertex> queue;

    visited[source] = true;
    outReachableSet.emplace_back(source);
    queue.emplace_back(source);

    while (!queue.empty()) {
        source = queue.front();
        queue.pop_front();

        for (auto vertex : graph.getConnected(source)) {
            if (!visited[vertex]) {
                visited[vertex] = true;

                outReachableSet.emplace_back(vertex);
                queue.emplace_back(vertex);
            }
        }
    }
}
