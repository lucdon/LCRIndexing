#pragma once

#include "PerLabelGraph.hpp"
#include "LabeledEdgeGraph.hpp"

class LabeledGraph {
private:
    std::vector<LabeledEdgeSet> adj;
    std::vector<LabeledEdgeSet> reverseAdj;

    size_t labelCount = 0;
    size_t edgeCount = 0;           // total number of unique (source, target, label) pairs.
    size_t labeledEdgeCount = 0;    // total number of unique (source, target) pairs.

public:
    LabeledGraph() = default;

    LabeledGraph(const LabeledGraph &) = delete;
    LabeledGraph(LabeledGraph &&) = default;

    LabeledGraph &operator =(const LabeledGraph &) = delete;
    LabeledGraph &operator =(LabeledGraph &&) = default;

    void setSizes(uint32_t vertices, uint32_t labels) {
        adj.resize(vertices);
        reverseAdj.resize(vertices);

        this->labelCount = labels;
    }

    void setEdgeSizes(uint32_t edges, uint32_t labeledEdges) {
        this->edgeCount = edges;
        this->labeledEdgeCount = labeledEdges;
    }

    [[nodiscard]] size_t getVertexCount() const {
        return adj.size();
    }

    [[nodiscard]] size_t getLabelCount() const {
        return labelCount;
    }

    [[nodiscard]] size_t getEdgeCount() const {
        return edgeCount;
    }

    [[nodiscard]] size_t getEdgeCount(Label label) const {
        auto count = 0ul;

        for (auto &targetList : adj) {
            for (auto &targets : targetList) {
                if (targets.second[label]) {
                    count++;
                }
            }
        }

        return count;
    }

    [[nodiscard]] size_t getLabeledEdgeCount() const {
        return labeledEdgeCount;
    }

    /**
     * @brief Assumes source, target pair are unique and valid. Does not update edgeCount and labeledEdgeCount.
     */
    void addEdgeNoChecks(Vertex source, Vertex target, const LabelSet &labels) {
        adj[source].emplace_back(target, labels);
        reverseAdj[target].emplace_back(source, labels);
    }

    /**
     * @brief Does not set adjacent edges and does not update edgeCount and labeledEdgeCount.
     */
    void setEdges(Vertex vertex, const std::unordered_map<Vertex, LabelSet> &adjVertices) {
        adj[vertex].reserve(adjVertices.size());

        for (auto &pair : adjVertices) {
            adj[vertex].emplace_back(pair.first, pair.second);
        }

        adj[vertex].shrink_to_fit();
    }

    /**
     * @brief Does not set reverse adjacent edges and does not update edgeCount and labeledEdgeCount.
     */
    void setRevEdges(Vertex vertex, const std::unordered_map<Vertex, LabelSet> &adjVertices) {
        reverseAdj[vertex].reserve(adjVertices.size());

        for (auto &pair : adjVertices) {
            reverseAdj[vertex].emplace_back(pair.first, pair.second);
        }

        reverseAdj[vertex].shrink_to_fit();
    }

    /**
     * @brief Does not set reverse adjacent edges and does not update edgeCount and labeledEdgeCount. Resets labelPerEdge labelSets.
     */
    void setEdgesNoChecks(Vertex source, const std::vector<Vertex> &edges, std::vector<LabelSet> &labelPerEdge) {
        adj[source].reserve(edges.size());

        for (auto target : edges) {
            adj[source].emplace_back(target, labelPerEdge[target]);
            labelPerEdge[target].reset();
        }
    }

    /**
     * @brief Does not set adjacent edges and does not update edgeCount and labeledEdgeCount. Resets labelPerEdge labelSets.
     */
    void setRevEdgesNoChecks(Vertex target, const std::vector<Vertex> &edges, std::vector<LabelSet> &labelPerEdge) {
        reverseAdj[target].reserve(edges.size());

        for (auto source : edges) {
            reverseAdj[target].emplace_back(source, labelPerEdge[source]);
            labelPerEdge[source].reset();
        }
    }

    [[nodiscard]] const LabeledEdgeSet &getConnected(Vertex source) const {
        return adj[source];
    }

    [[nodiscard]] const LabeledEdgeSet &getReverseConnected(Vertex target) const {
        return reverseAdj[target];
    }

    void optimize() {

    }
};

void labelDistribution(const LabeledGraph &labeledGraph, std::vector<std::pair<uint32_t, Label>> &distribution);
void vertexDistribution(const LabeledGraph &labeledGraph, std::vector<std::pair<uint32_t, Vertex>> &distribution);

void orderLabelsByFrequency(const LabeledGraph &labeledGraph, std::vector<Label> &order);

std::ostream &printLabelDistribution(std::ostream &out, const LabeledGraph &perLabelGraph);
std::ostream &printVertexDistribution(std::ostream &out, const LabeledGraph &labeledGraph);

std::ostream &operator <<(std::ostream &out, const LabeledGraph &labeledGraph);
