#pragma once

#include "DiGraph.hpp"

/**
 * @brief Not used anymore, does not work for large graphs since it requires to load |L| x |V| std::vectors.
 * Which could be several GBs while the graph itself is only a few MBs.
 */
class PerLabelGraph {
private:
    std::vector<DiGraph> graphs;

    uint32_t vertexCount = 0;
    uint32_t labelCount = 0;
    uint32_t edgeCount = 0;

public:
    PerLabelGraph() = default;

    PerLabelGraph(const PerLabelGraph &) = delete;
    PerLabelGraph(PerLabelGraph &&) = default;

    PerLabelGraph &operator =(const PerLabelGraph &) = delete;
    PerLabelGraph &operator =(PerLabelGraph &&) = default;

    void setSizes(uint32_t vertices, uint32_t labels) {
        vertexCount = vertices;
        labelCount = labels;

        graphs.resize(labelCount);

        for (auto &graph : graphs) {
            graph.setVertices(vertexCount);
        }
    }

    /**
     * @brief Add (source, target, label) as edge.
     * It will only add the edge if it has not already been added.
     * It will crash the application if the edge is not within the graphs bounds.
     */
    void addEdge(Vertex source, Vertex target, Label label) {
        if (label >= graphs.size()) {
            std::cerr << "Failed adding edge, label out of bounds! source: " << source << " target: " << target
                      << " label: " << label << " max: " << graphs.size() << std::fatal;
        }

        if (graphs[label].addEdge(source, target)) {
            edgeCount++;
        }
    }

    /**
     * @brief Add (source, target, label) as edge. Does not perform checks.
     * Thus assumes: edge is unique and within bounds.
     */
    void addEdgeNoChecks(Vertex source, Vertex target, Label label) {
        graphs[label].addEdgeNoChecks(source, target);
        edgeCount++;
    }

    /**
     * @brief Add a list of edges starting from source. Does not perform checks.
     * Thus assumes: all edge are unique and within bounds.
     */
    void addEdgesNoChecks(Vertex source, Label label, std::vector<Vertex> targets) {
        graphs[label].addEdgesNoChecks(source, targets);
        edgeCount++;
    }

    [[nodiscard]] uint32_t getVertexCount() const {
        return vertexCount;
    }

    [[nodiscard]] uint32_t getLabelCount() const {
        return labelCount;
    }

    [[nodiscard]] uint32_t getEdgeCount() const {
        return edgeCount;
    }

    [[nodiscard]] uint32_t getEdgeCount(Label label) const {
        return graphs[label].getEdgeCount();
    }

    void optimize() {
        for (auto &graph : graphs) {
            graph.optimize();
        }
    }

    [[nodiscard]] bool hasConnected(Vertex source) const {
        for (auto &graph : graphs) {
            if (!graph.getConnected(source).empty()) {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] bool hasReverseConnected(Vertex source) const {
        for (auto &graph : graphs) {
            if (!graph.getReverseConnected(source).empty()) {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] const std::vector<Vertex> &getConnected(Vertex source, Label label) const {
        return graphs[label].getConnected(source);
    }

    [[nodiscard]] const std::vector<Vertex> &getReverseConnected(Vertex target, Label label) const {
        return graphs[label].getReverseConnected(target);
    }
};

void labelDistribution(const PerLabelGraph &perLabelGraph, std::vector<std::pair<uint32_t, Label>> &distribution);
void orderLabelsByFrequency(const PerLabelGraph &perLabelGraph, std::vector<Label> &order);
std::ostream &printLabelDistribution(std::ostream &out, const PerLabelGraph &perLabelGraph);
std::ostream &operator <<(std::ostream &out, const PerLabelGraph &perLabelGraph);