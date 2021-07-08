#include <graphs/Graph.hpp>
#include "graphs/SCCGraph.hpp"



void iterativeDFS(const Graph &graph, std::stack<Vertex> &vertexStack, std::stack<Vertex> &iteratorStack,
                  std::stack<Vertex> &tarjanStack, boost::dynamic_bitset<> &tarjanVisited, std::vector<Vertex> &discovery,
                  std::vector<uint32_t> &lowLink, uint32_t &componentCount, uint32_t &currentIndex, Vertex start);

uint32_t tarjanWCC(const DiGraph &graph) {
    uint32_t componentCount = 0;
    uint32_t currentIndex = 1;

    auto undirectedGraph = diGraphToGraph(graph);

    std::vector<Vertex> discovery(graph.getVertexCount());
    std::vector<uint32_t> lowLink(graph.getVertexCount());

    boost::dynamic_bitset<> tarjanVisited(graph.getVertexCount());
    std::stack<Vertex> tarjanStack;

    std::stack<Vertex> vertexStack;
    std::stack<Vertex> iteratorStack;

    for (auto vertex = 0; vertex < graph.getVertexCount(); vertex++) {
        if (discovery[vertex] == 0) {
            // Perform iterative dfs.
            iterativeDFS(*undirectedGraph, vertexStack, iteratorStack, tarjanStack, tarjanVisited, discovery, lowLink,
                         componentCount, currentIndex, vertex);
        }
    }

    return componentCount;
}

void iterativeDFS(const Graph &graph, std::stack<Vertex> &vertexStack, std::stack<Vertex> &iteratorStack,
                  std::stack<Vertex> &tarjanStack, boost::dynamic_bitset<> &tarjanVisited, std::vector<Vertex> &discovery,
                  std::vector<uint32_t> &lowLink, uint32_t &componentCount, uint32_t &currentIndex, Vertex start) {

    discovery[start] = currentIndex;
    lowLink[start] = currentIndex;

    currentIndex++;

    tarjanStack.push(start);
    tarjanVisited[start] = true;

    vertexStack.push(start);
    iteratorStack.push(0);

    while (true) {
        auto vertex = vertexStack.top();
        auto &iteratorPointer = iteratorStack.top();
        auto &adjacencyList = graph.getConnected(vertex);

        if (iteratorPointer < adjacencyList.size()) {
            //  We have not yet finished the current iterator.
            auto adjVertex = adjacencyList[iteratorPointer++];

            if (discovery[adjVertex] == 0) {
                // Move one up on the stack.
                discovery[adjVertex] = currentIndex;
                lowLink[adjVertex] = currentIndex;

                currentIndex++;

                tarjanStack.push(adjVertex);
                tarjanVisited[adjVertex] = true;

                vertexStack.push(adjVertex);
                iteratorStack.push(0);
            } else if (tarjanVisited[adjVertex]) {
                lowLink[vertex] = std::min(lowLink[vertex], discovery[adjVertex]);
            }
        } else {
            // Process backtracking.
            if (lowLink[vertex] == discovery[vertex]) {
                Vertex connectedVertex;
                componentCount++;

                do {
                    connectedVertex = tarjanStack.top();
                    tarjanStack.pop();

                    tarjanVisited[connectedVertex] = false;
                } while (vertex != connectedVertex && !tarjanStack.empty());
            }

            vertexStack.pop();
            iteratorStack.pop();

            if (iteratorStack.empty()) {
                break;
            }

            // revisit so update lowLink.
            auto newVertex = vertexStack.top();
            lowLink[newVertex] = std::min(lowLink[newVertex], lowLink[vertex]);
        }
    }
}
