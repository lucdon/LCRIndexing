#include "graphs/SCCGraph.hpp"

bool reachabilityBiBFS(const DiGraph &graph, Vertex source, Vertex target) {
    if (source == target) {
        return true;
    }

    boost::dynamic_bitset<> visitedSource(graph.getVertexCount());
    boost::dynamic_bitset<> visitedTarget(graph.getVertexCount());

    std::deque<Vertex> queueSource;
    std::deque<Vertex> queueTarget;

    visitedSource[source] = true;
    visitedTarget[target] = true;

    queueSource.emplace_back(source);
    queueTarget.emplace_back(target);

    while (!queueSource.empty() && !queueTarget.empty()) {
        source = queueSource.front();
        queueSource.pop_front();

        for (auto vertex : graph.getConnected(source)) {
            if (!visitedSource[vertex]) {
                visitedSource[vertex] = true;

                if(visitedTarget[vertex]) {
                    return true;
                }

                queueSource.emplace_back(vertex);
            }
        }

        target = queueTarget.front();
        queueTarget.pop_front();

        for (auto vertex : graph.getReverseConnected(target)) {
            if (!visitedTarget[vertex]) {
                visitedTarget[vertex] = true;

                if(visitedSource[vertex]) {
                    return true;
                }

                queueTarget.emplace_back(vertex);
            }
        }
    }

    return false;
}