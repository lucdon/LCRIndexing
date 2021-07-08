#include "graphs/LabeledGraph.hpp"
#include "utility/MemoryWatch.hpp"

std::unique_ptr<LabeledGraph>
createLabeledGraph(std::vector<std::tuple<Vertex, Vertex, Label>> &edges, uint32_t vertexCount, uint32_t labelCount) {

    auto graph = std::make_unique<LabeledGraph>();
    graph->setSizes(vertexCount, labelCount);

    std::unordered_map<Vertex, LabelSet> adj;

    adj.reserve(vertexCount);

    struct source_pred {
        constexpr bool operator ()(std::tuple<Vertex, Vertex, Label> const &left,
                                   std::tuple<Vertex, Vertex, Label> const &right) const {
            if (std::get<0>(left) < std::get<0>(right)) {
                return true;
            }

            if (std::get<0>(left) > std::get<0>(right)) {
                return false;
            }

            return std::get<1>(left) < std::get<1>(right);
        }
    };

    uint32_t currentVertex = 0;
    uint32_t edgeCount = 0;
    uint32_t labeledEdgeCount = 0;

    std::sort(edges.begin(), edges.end(), source_pred());

    formatMemory(std::cout, getCurrentPSS());
    formatMemory(std::cout, getCurrentRSS());
    std::cout << std::endl;

    MemoryWatch memoryWatch;
    memoryWatch.begin("load adj");

    for (auto &edge : edges) {
        auto source = std::get<0>(edge);
        auto target = std::get<1>(edge);
        auto label = std::get<2>(edge);

        while (source > currentVertex) {
            graph->setEdges(currentVertex, adj);
            adj.clear();
            currentVertex++;
        }

        if (adj[target].empty()) {
            adj[target].resize(labelCount);
            labeledEdgeCount++;
        }

        if (!adj[target][label]) {
            edgeCount++;
        }

        adj[target][label] = true;
    }

    graph->setEdges(currentVertex, adj);
    adj.clear();
    graph->setEdgeSizes(edgeCount, labeledEdgeCount);

    memoryWatch.end();

    struct target_pred {
        constexpr bool operator ()(std::tuple<Vertex, Vertex, Label> const &left,
                                   std::tuple<Vertex, Vertex, Label> const &right) const {
            if (std::get<1>(left) < std::get<1>(right)) {
                return true;
            }

            if (std::get<1>(left) > std::get<1>(right)) {
                return false;
            }

            return std::get<0>(left) < std::get<0>(right);
        }
    };

    std::sort(edges.begin(), edges.end(), target_pred());
    currentVertex = 0;

    memoryWatch.begin("load rev adj");

    for (auto &edge : edges) {
        auto source = std::get<0>(edge);
        auto target = std::get<1>(edge);
        auto label = std::get<2>(edge);

        while (target > currentVertex) {
            graph->setRevEdges(currentVertex, adj);
            adj.clear();
            currentVertex++;
        }

        if (adj[source].empty()) {
            adj[source].resize(labelCount);
        }

        adj[source][label] = true;
    }

    graph->setRevEdges(currentVertex, adj);

    memoryWatch.end();
    return graph;
}