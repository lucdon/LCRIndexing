#include "graphs/SCCGraph.hpp"

bool BFS(const DiGraph &graph, Vertex source, Vertex target, Path &outPath) {
    if (source == target) {
        outPath.emplace_back(source);
        return true;
    }

    boost::dynamic_bitset<> visited(graph.getVertexCount());
    std::vector<Vertex> parents(graph.getVertexCount());
    std::deque<Vertex> queue;

    visited[source] = true;
    queue.emplace_back(source);
    bool found = false;
    uint32_t depth = 0;

    while (!queue.empty() && !found) {
        depth++;
        auto newSource = queue.front();
        queue.pop_front();

        for (auto vertex : graph.getConnected(newSource)) {
            if (vertex == target) {
                parents[vertex] = newSource;
                found = true;
                break;
            }

            if (!visited[vertex]) {
                visited[vertex] = true;
                parents[vertex] = newSource;

                queue.emplace_back(vertex);
            }
        }
    }

    if (!found) {
        return false;
    }

    for (auto i = 0u; i < depth; i++) {
        auto parent = parents[target];
        outPath.emplace_back(target);
        target = parent;
    }

    outPath.emplace_back(source);

    std::reverse(outPath.begin(), outPath.end());
    return true;
}