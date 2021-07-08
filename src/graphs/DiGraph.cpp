#include "DiGraph.hpp"

std::ostream &operator <<(std::ostream &out, const Path &path) {
    if (path.empty()) {
        return out;
    }

    out << path[0];

    for (auto i = 1u; i < path.size(); i++) {
        out << " -> " << path[i];
    }

    return out;
}

std::ostream &operator <<(std::ostream &out, const DiGraph &graph) {
    out << "G = (" << graph.getVertexCount() << ", " << graph.getEdgeCount() << ")";
    return out;
}

void printStats(std::ostream &out, const DiGraph &graph) {
    std::vector<std::pair<size_t, Vertex>> incomingDegreeByVertex(graph.getVertexCount());
    std::vector<std::pair<size_t, Vertex>> outgoingDegreeByVertex(graph.getVertexCount());

    for (int vertex = 0u; vertex < graph.getVertexCount(); vertex++) {
        uint32_t outgoingDegree = graph.getConnected(vertex).size();
        outgoingDegreeByVertex[vertex] = std::make_pair(outgoingDegree, vertex);

        uint32_t incomingDegree = graph.getReverseConnected(vertex).size();
        incomingDegreeByVertex[vertex] = std::make_pair(incomingDegree, vertex);
    }

    auto numWeaklyConnected = tarjanWCC(graph);

    std::sort(incomingDegreeByVertex.begin(), incomingDegreeByVertex.end(), std::greater<>());
    std::sort(outgoingDegreeByVertex.begin(), outgoingDegreeByVertex.end(), std::greater<>());

    auto q25th = (uint32_t) ((1 - 0.25) * (double) graph.getVertexCount());
    auto q50th = (uint32_t) ((1 - 0.5) * (double) graph.getVertexCount());
    auto q75th = (uint32_t) ((1 - 0.75) * (double) graph.getVertexCount());
    auto q95th = (uint32_t) ((1 - 0.95) * (double) graph.getVertexCount());
    auto q99th = (uint32_t) ((1 - 0.99) * (double) graph.getVertexCount());

    out << "G                = (" << graph.getVertexCount() << ", " << graph.getEdgeCount() << ")\n";
    out << "WCC count        = " << numWeaklyConnected << "\n\n";

    out << "indegree first   = " << incomingDegreeByVertex[0].first << " at: " << incomingDegreeByVertex[0].second
        << "\n";
    out << "indegree second  = " << incomingDegreeByVertex[1].first << " at: " << incomingDegreeByVertex[1].second
        << "\n";
    out << "indegree third   = " << incomingDegreeByVertex[2].first << " at: " << incomingDegreeByVertex[2].second
        << "\n";
    out << "indegree 99%     = " << incomingDegreeByVertex[q99th].first << "\n";
    out << "indegree 95%     = " << incomingDegreeByVertex[q95th].first << "\n";
    out << "indegree 75%     = " << incomingDegreeByVertex[q75th].first << "\n";
    out << "indegree 50%     = " << incomingDegreeByVertex[q50th].first << "\n";
    out << "indegree 25%     = " << incomingDegreeByVertex[q25th].first << "\n";
    out << "indegree min     = " << incomingDegreeByVertex.back().first << "\n\n";

    out << "outdegree first  = " << outgoingDegreeByVertex[0].first << " at: " << outgoingDegreeByVertex[0].second
        << "\n";
    out << "outdegree second = " << outgoingDegreeByVertex[1].first << " at: " << outgoingDegreeByVertex[1].second
        << "\n";
    out << "outdegree third  = " << outgoingDegreeByVertex[2].first << " at: " << outgoingDegreeByVertex[2].second
        << "\n";
    out << "outdegree 99%    = " << outgoingDegreeByVertex[q99th].first << "\n";
    out << "outdegree 95%    = " << outgoingDegreeByVertex[q95th].first << "\n";
    out << "outdegree 75%    = " << outgoingDegreeByVertex[q75th].first << "\n";
    out << "outdegree 50%    = " << outgoingDegreeByVertex[q50th].first << "\n";
    out << "outdegree 25%    = " << outgoingDegreeByVertex[q25th].first << "\n";
    out << "outdegree min    = " << outgoingDegreeByVertex.back().first << "\n";
}