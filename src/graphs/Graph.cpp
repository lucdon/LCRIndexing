#include "Graph.hpp"

std::ostream &operator <<(std::ostream &out, const Graph &graph) {
    out << "G = (" << graph.getVertexCount() << ", " << graph.getEdgeCount() << ")";
    return out;
}