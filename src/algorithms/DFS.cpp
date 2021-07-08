#include "graphs/SCCGraph.hpp"

bool DFS(const DiGraph &graph, Vertex source, Vertex target, Path &outPath) {
    if (source == target) {
        outPath.emplace_back(source);
        return true;
    }

    boost::dynamic_bitset<> visited(graph.getVertexCount());
    std::vector<Vertex> parents(graph.getVertexCount());
    std::deque<Vertex> stack;

    visited[source] = true;
    stack.emplace_back(source);
    bool found = false;
    uint32_t depth = 0;

    while (!stack.empty() && !found) {
        depth++;
        auto newSource = stack.back();
        stack.pop_back();

        for (auto vertex : graph.getConnected(newSource)) {
            if (vertex == target) {
                parents[vertex] = newSource;
                found = true;
                break;
            }

            if (!visited[vertex]) {
                visited[vertex] = true;
                parents[vertex] = newSource;

                stack.emplace_back(vertex);
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