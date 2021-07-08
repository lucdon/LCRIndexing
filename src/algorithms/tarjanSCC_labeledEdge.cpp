#include "graphs/SCCGraph.hpp"
#include "graphs/LabeledGraph.hpp"

void iterativeDFS(const LabeledEdgeGraph &graph, std::stack<Vertex> &vertexStack, std::stack<LabeledEdgeGraphIterator> &iteratorStack,
                  std::stack<Vertex> &tarjanStack, boost::dynamic_bitset<> &tarjanVisited, std::vector<Vertex> &discovery,
                  std::vector<uint32_t> &lowLink, std::vector<std::vector<Vertex>> &components,
                  std::vector<uint32_t> &vertexMapping, uint32_t &currentIndex, Vertex start);

std::unique_ptr<SCCGraph> tarjanSCC(const LabeledEdgeGraph &graph) {
    return tarjanSCC(graph, false);
}

std::unique_ptr<SCCGraph> tarjanSCC(const LabeledEdgeGraph &graph, bool includeComponents) {
    std::vector<std::vector<Vertex>> components;
    std::vector<Vertex> vertexMapping(graph.getVertexCount());

    uint32_t currentIndex = 1;

    std::vector<Vertex> discovery(graph.getVertexCount());
    std::vector<uint32_t> lowLink(graph.getVertexCount());

    boost::dynamic_bitset<> tarjanVisited(graph.getVertexCount());

    std::stack<Vertex> tarjanStack;

    std::stack<Vertex> vertexStack;
    std::stack<LabeledEdgeGraphIterator> iteratorStack;

    components.reserve(graph.getVertexCount());

    for (auto vertex = 0; vertex < graph.getVertexCount(); vertex++) {
        if (discovery[vertex] == 0) {
            // Perform iterative dfs.
            iterativeDFS(graph, vertexStack, iteratorStack, tarjanStack, tarjanVisited, discovery, lowLink,
                         components, vertexMapping, currentIndex, vertex);
        }
    }

    auto componentGraph = std::make_unique<DiGraph>();
    componentGraph->setVertices(components.size());

    std::vector<int> edgeUsed(components.size(), -1);
    std::vector<Vertex> targets;

    for (auto sourceComponent = 0; sourceComponent < components.size(); sourceComponent++) {
        for (auto source : components[sourceComponent]) {

            auto it = graph.getConnected(source);

            while(it.next()) {
                auto& edge = *it;

                auto targetComponent = vertexMapping[edge.target];

                if (sourceComponent == targetComponent) {
                    continue;
                }

                if (edgeUsed[targetComponent] == sourceComponent) {
                    continue;
                }

                edgeUsed[targetComponent] = sourceComponent;
                targets.emplace_back(targetComponent);
            }
        }

        componentGraph->addEdgesNoChecks(sourceComponent, targets);
        targets.clear();
    }

    componentGraph->optimize();

    if (includeComponents) {
        components.shrink_to_fit();
        return std::make_unique<SCCGraph>(std::move(componentGraph), std::move(vertexMapping), std::move(components));
    }

    return std::make_unique<SCCGraph>(std::move(componentGraph), std::move(vertexMapping));
}

void iterativeDFS(const LabeledEdgeGraph &graph, std::stack<Vertex> &vertexStack, std::stack<LabeledEdgeGraphIterator> &iteratorStack,
                  std::stack<Vertex> &tarjanStack, boost::dynamic_bitset<> &tarjanVisited, std::vector<Vertex> &discovery,
                  std::vector<uint32_t> &lowLink, std::vector<std::vector<Vertex>> &components,
                  std::vector<uint32_t> &vertexMapping, uint32_t &currentIndex, Vertex start) {

    discovery[start] = currentIndex;
    lowLink[start] = currentIndex;

    currentIndex++;

    tarjanStack.push(start);
    tarjanVisited[start] = true;

    vertexStack.push(start);
    iteratorStack.push(graph.getConnected(start));

    while (true) {
        auto vertex = vertexStack.top();
        auto &iteratorPointer = iteratorStack.top();

        if (iteratorPointer.next()) {
            //  We have not yet finished the current iterator.
            auto& edge = *iteratorPointer;
            auto adjVertex = edge.target;

            if (discovery[adjVertex] == 0) {
                // Move one up on the stack.
                discovery[adjVertex] = currentIndex;
                lowLink[adjVertex] = currentIndex;

                currentIndex++;

                tarjanStack.push(adjVertex);
                tarjanVisited[adjVertex] = true;

                vertexStack.push(adjVertex);
                iteratorStack.push(graph.getConnected(adjVertex));
            } else if (tarjanVisited[adjVertex]) {
                lowLink[vertex] = std::min(lowLink[vertex], discovery[adjVertex]);
            }
        } else {
            // Process backtracking.
            if (lowLink[vertex] == discovery[vertex]) {
                Vertex connectedVertex;

                auto &component = components.emplace_back();

                do {
                    connectedVertex = tarjanStack.top();
                    tarjanStack.pop();

                    tarjanVisited[connectedVertex] = false;
                    component.push_back(connectedVertex);
                    vertexMapping[connectedVertex] = components.size() - 1;
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
