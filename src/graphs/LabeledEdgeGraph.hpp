#pragma once

#include "PerLabelGraph.hpp"

#define LABELED_EDGE_GRAPH_LABEL_SORTED 0

class LabeledEdgeGraphIterator {
private:
    const std::vector<Edge> &edges;

    size_t numTotal;
    uint32_t startIndex;
    uint32_t currentIndex;
    uint32_t source;

    bool started = false;

public:
    LabeledEdgeGraphIterator(const std::vector<Edge> &edges, uint32_t currentIndex, size_t numTotal,
                             uint32_t source) : edges(edges), currentIndex(currentIndex), startIndex(currentIndex),
                                                source(source), numTotal(numTotal) { }

    [[nodiscard]]bool next() {
        if (started) {
            ++currentIndex;
        } else {
            started = true;
        }

        return isValid();
    }

    [[nodiscard]]bool isValid() const {
        return currentIndex < edges.size() && edges[currentIndex].source == source;
    }

    void reset() {
        currentIndex = startIndex;
        started = false;
    }

    [[nodiscard]] size_t size() const {
        return numTotal;
    }

    const Edge &operator [](uint32_t index) const {
        return edges[startIndex + index];
    }

    const Edge &operator *() const {
        return edges[currentIndex];
    }

    const Edge *operator ->() const {
        return &edges[currentIndex];
    }
};

class LabeledEdgeGraphLabelIterator {
private:
    const std::vector<Edge> &edges;
    uint32_t currentIndex;

    bool started = false;
    uint32_t source;
    uint32_t label;

public:
    LabeledEdgeGraphLabelIterator(const std::vector<Edge> &edges, uint32_t currentIndex, uint32_t source,
                                  uint32_t label) : edges(edges), currentIndex(currentIndex), source(source),
                                                    label(label) { }

    [[nodiscard]]bool next() {
#if LABELED_EDGE_GRAPH_LABEL_SORTED == 1
        if (started) {
            ++currentIndex;
        } else {
            started = true;
        }

        while (currentIndex < edges.size() && edges[currentIndex].source == source) {
            if (edges[currentIndex].label < label) {
                ++currentIndex;
                continue;
            }

            if (edges[currentIndex].label > label) {
                return false;
            }

            return true;
        }

        return false;
#else
        if (started) {
            ++currentIndex;
        } else {
            started = true;
        }

        while (currentIndex < edges.size() && edges[currentIndex].source == source &&
               edges[currentIndex].label != label) {
            ++currentIndex;
        }

        return isValid();
#endif
    }

    [[nodiscard]]bool isValid() const {
        return currentIndex < edges.size() && edges[currentIndex].source == source &&
               edges[currentIndex].label == label;
    }

    const Edge &operator *() const {
        return edges[currentIndex];
    }

    const Edge *operator ->() const {
        return &edges[currentIndex];
    }
};

class LabeledEdgeGraphLabelSetIterator {
private:
    const std::vector<Edge> &edges;
    uint32_t currentIndex;

    bool started = false;
    uint32_t source;
    const LabelSet &labelSet;

public:
    LabeledEdgeGraphLabelSetIterator(const std::vector<Edge> &edges, uint32_t currentIndex, uint32_t source,
                                     const LabelSet &labelSet) : edges(edges), currentIndex(currentIndex),
                                                                 source(source), labelSet(labelSet) { }

    [[nodiscard]]bool next() {
        if (started) {
            ++currentIndex;
        } else {
            started = true;
        }

        while (currentIndex < edges.size() && edges[currentIndex].source == source &&
               !labelSet[edges[currentIndex].label]) {
            ++currentIndex;
        }

        return isValid();
    }

    [[nodiscard]]bool isValid() const {
        return currentIndex < edges.size() && edges[currentIndex].source == source &&
               labelSet[edges[currentIndex].label];
    }

    const Edge &operator *() const {
        return edges[currentIndex];
    }

    const Edge *operator ->() const {
        return &edges[currentIndex];
    }
};

class LabeledEdgeGraph {
private:
    std::vector<Edge> adj;
    std::vector<Edge> reverseAdj;

    std::vector<uint32_t> adjStartLookup;
    std::vector<uint32_t> reverseAdjStartLookup;

    size_t vertexCount = 0;
    size_t labelCount = 0;

public:
    LabeledEdgeGraph() = default;

    LabeledEdgeGraph(const LabeledEdgeGraph &) = delete;
    LabeledEdgeGraph(LabeledEdgeGraph &&) = default;

    LabeledEdgeGraph &operator =(const LabeledEdgeGraph &) = delete;
    LabeledEdgeGraph &operator =(LabeledEdgeGraph &&) = default;

    void setSizes(uint32_t vertices, uint32_t labels, uint32_t edges) {
        adjStartLookup.resize(vertices);
        reverseAdjStartLookup.resize(vertices);

        this->adj.reserve(edges);
        this->reverseAdj.reserve(edges);

        this->vertexCount = vertices;
        this->labelCount = labels;
    }

    [[nodiscard]] size_t getVertexCount() const {
        return vertexCount;
    }

    [[nodiscard]] size_t getLabelCount() const {
        return labelCount;
    }

    [[nodiscard]] size_t getEdgeCount() const {
        return adj.size();
    }

    [[nodiscard]] size_t getEdgeCount(Label label) const {
        auto count = 0ul;

        for (auto &edge : adj) {
            if (edge.label == label) {
                count++;
            }
        }

        return count;
    }

    void addEdge(Vertex source, Vertex target, Label label) {
        adj.emplace_back(source, target, label);
        reverseAdj.emplace_back(target, source, label);
    }

    [[nodiscard]] LabeledEdgeGraphIterator getConnected(Vertex source) const {
        if (source == vertexCount - 1) {
            return LabeledEdgeGraphIterator(adj, adjStartLookup[source], getEdgeCount() - adjStartLookup[source],
                                            source);
        }

        return LabeledEdgeGraphIterator(adj, adjStartLookup[source],
                                        adjStartLookup[source + 1] - adjStartLookup[source], source);
    }

    [[nodiscard]] LabeledEdgeGraphLabelIterator getConnected(Vertex source, uint32_t label) const {
        return LabeledEdgeGraphLabelIterator(adj, adjStartLookup[source], source, label);
    }

    [[nodiscard]] LabeledEdgeGraphLabelSetIterator getConnected(Vertex source, const LabelSet &labelSet) const {
        return LabeledEdgeGraphLabelSetIterator(adj, adjStartLookup[source], source, labelSet);
    }

    [[nodiscard]] LabeledEdgeGraphIterator getReverseConnected(Vertex source) const {
        if (source == vertexCount - 1) {
            return LabeledEdgeGraphIterator(reverseAdj, reverseAdjStartLookup[source],
                                            getEdgeCount() - reverseAdjStartLookup[source], source);
        }

        return LabeledEdgeGraphIterator(reverseAdj, reverseAdjStartLookup[source],
                                        reverseAdjStartLookup[source + 1] - reverseAdjStartLookup[source], source);
    }

    [[nodiscard]] LabeledEdgeGraphLabelIterator getReverseConnected(Vertex source, uint32_t label) const {
        return LabeledEdgeGraphLabelIterator(reverseAdj, reverseAdjStartLookup[source], source, label);
    }

    [[nodiscard]] LabeledEdgeGraphLabelSetIterator getReverseConnected(Vertex source, const LabelSet &labelSet) const {
        return LabeledEdgeGraphLabelSetIterator(reverseAdj, reverseAdjStartLookup[source], source, labelSet);
    }

    void optimize() {
        struct source_target_label_pred {
            // First sort by source then by target and finally on label.
            constexpr bool operator ()(Edge const &left, Edge const &right) const {
                if (left.source < right.source) {
                    return true;
                }

                if (left.source > right.source) {
                    return false;
                }

                if (left.target < right.target) {
                    return true;
                }

                if (left.target > right.target) {
                    return false;
                }

                return left.label < right.label;
            }
        };

        struct source_label_target_pred {
            // First sort by source then by label and finally on target.
            constexpr bool operator ()(Edge const &left, Edge const &right) const {
                if (left.source < right.source) {
                    return true;
                }

                if (left.source > right.source) {
                    return false;
                }

                if (left.label < right.label) {
                    return true;
                }

                if (left.label > right.label) {
                    return false;
                }

                return left.target < right.target;
            }
        };

        struct source_eq {
            // First sort by source then by target and finally on label.
            constexpr bool operator ()(Edge const &left, Edge const &right) const {
                return left.source == right.source && left.target == right.target && left.label == right.label;
            }
        };

#if LABELED_EDGE_GRAPH_LABEL_SORTED == 1
        std::sort(adj.begin(), adj.end(), source_label_target_pred());
#else
        std::sort(adj.begin(), adj.end(), source_target_label_pred());
#endif

        auto last = std::unique(adj.begin(), adj.end(), source_eq());
        adj.erase(last, adj.end());
        adj.shrink_to_fit();

        auto currentSource = 0u;
        auto currentPointer = 0u;

        adjStartLookup[currentSource] = currentPointer;

        for (auto &edge : adj) {
            auto source = edge.source;

            while (source >= currentSource) {
                adjStartLookup[currentSource] = currentPointer;
                currentSource++;
            }

            currentPointer++;
        }

        while (currentSource < vertexCount) {
            adjStartLookup[currentSource] = currentPointer;
            currentSource++;
        }


#if LABELED_EDGE_GRAPH_LABEL_SORTED == 1
        std::sort(reverseAdj.begin(), reverseAdj.end(), source_label_target_pred());
#else
        std::sort(reverseAdj.begin(), reverseAdj.end(), source_target_label_pred());
#endif

        last = std::unique(reverseAdj.begin(), reverseAdj.end(), source_eq());
        reverseAdj.erase(last, reverseAdj.end());
        reverseAdj.shrink_to_fit();

        currentSource = 0;
        currentPointer = 0;

        reverseAdjStartLookup[currentSource] = currentPointer;

        for (auto &edge : reverseAdj) {
            auto source = edge.source;

            while (source >= currentSource) {
                reverseAdjStartLookup[currentSource] = currentPointer;
                currentSource++;
            }

            currentPointer++;
        }

        while (currentSource < vertexCount) {
            reverseAdjStartLookup[currentSource] = currentPointer;
            currentSource++;
        }
    }
};

void labelDistribution(const LabeledEdgeGraph &labeledGraph, std::vector<std::pair<uint32_t, Label>> &distribution);
void vertexDistribution(const LabeledEdgeGraph &labeledGraph, std::vector<std::pair<uint32_t, Vertex>> &distribution);

void orderLabelsByFrequency(const LabeledEdgeGraph &labeledGraph, std::vector<Label> &order);

std::ostream &printLabelDistribution(std::ostream &out, const LabeledEdgeGraph &perLabelGraph);
std::ostream &printVertexDistribution(std::ostream &out, const LabeledEdgeGraph &labeledGraph);

std::ostream &operator <<(std::ostream &out, const LabeledEdgeGraph &labeledGraph);
