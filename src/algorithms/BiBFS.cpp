#include "graphs/SCCGraph.hpp"

bool BiBFS(const DiGraph &graph, Vertex source, Vertex target, Path &outPath) {
    if (source == target) {
        outPath.emplace_back(source);
        return true;
    }

    boost::dynamic_bitset<> visitedSource(graph.getVertexCount());
    boost::dynamic_bitset<> visitedTarget(graph.getVertexCount());

    std::vector<Vertex> parentsSource(graph.getVertexCount());
    std::vector<Vertex> parentsTarget(graph.getVertexCount());

    std::deque<Vertex> queueSource;
    std::deque<Vertex> queueTarget;

    visitedSource[source] = true;
    visitedTarget[target] = true;

    queueSource.emplace_back(source);
    queueTarget.emplace_back(target);

    uint32_t depth = 0;
    Vertex matchingVertex = 0;
    bool found = false;

    while (!queueSource.empty() && !queueTarget.empty()) {
        depth++;
        auto newSource = queueSource.front();
        queueSource.pop_front();

        for (auto vertex : graph.getConnected(newSource)) {
            if (!visitedSource[vertex]) {
                visitedSource[vertex] = true;
                parentsSource[vertex] = newSource;
                queueSource.emplace_back(vertex);

                if (visitedTarget[vertex]) {
                    found = true;
                    break;
                }
            }
        }

        if (found) {
            break;
        }

        auto newTarget = queueTarget.front();
        queueTarget.pop_front();

        for (auto vertex : graph.getReverseConnected(newTarget)) {
            if (!visitedTarget[vertex]) {
                visitedTarget[vertex] = true;
                parentsTarget[vertex] = newTarget;
                queueTarget.emplace_back(vertex);

                if (visitedSource[vertex]) {
                    found = true;
                    break;
                }
            }
        }

        if (found) {
            break;
        }
    }

    if (!found) {
        return false;
    }

    auto middleVertex = matchingVertex;

    while (middleVertex != source) {
        middleVertex = parentsSource[middleVertex];
        outPath.emplace_back(middleVertex);
    }

    std::reverse(outPath.begin(), outPath.end());

    while (matchingVertex != target) {
        outPath.emplace_back(matchingVertex);
        matchingVertex = parentsTarget[matchingVertex];
    }

    outPath.emplace_back(target);

    return true;
}