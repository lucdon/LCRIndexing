#pragma once

#include <utility>

#include "graphs/SCCGraph.hpp"
#include "LabeledGraph.hpp"

/**
 * Returns true iff target is reachable from source.
 */
struct ReachQuery {
public:
    Vertex source;
    Vertex target;

    ReachQuery() = default;
    ReachQuery(const ReachQuery &) = default;
    ReachQuery(ReachQuery &&) = default;

    ReachQuery &operator =(const ReachQuery &) = default;

    ReachQuery(Vertex source, Vertex target) : source(source), target(target) { }
};

/**
 * Returns true iff target is reachable from source constraint by a set of labels.
 */
struct LCRQuery {
public:
    Vertex source = 0;
    Vertex target = 0;

    LabelSet labelSet;
    std::vector<Label> labels;

    LCRQuery() = default;
    LCRQuery(const LCRQuery &) = default;
    LCRQuery(LCRQuery &&) = default;

    LCRQuery &operator =(const LCRQuery &query) = default;

    /**
     * @brief Creates a LCRQuery. Requires init to be called, after creating the query.
     */
    LCRQuery(Vertex source, Vertex target, std::vector<Label> labels) : source(source), target(target),
                                                                        labels(std::move(labels)) { }

    void init(const LabeledEdgeGraph &graph) {
        if (labelSet.size() == graph.getLabelCount()) {
            return;
        }

        labelSet.resize(graph.getLabelCount());

        for (auto label : labels) {
            labelSet[label] = true;
        }
    }

    void init(const LabeledGraph &graph) {
        if (labelSet.size() == graph.getLabelCount()) {
            return;
        }

        labelSet.resize(graph.getLabelCount());

        for (auto label : labels) {
            labelSet[label] = true;
        }
    }

    void init(const PerLabelGraph &graph) {
        if (labelSet.size() == graph.getLabelCount()) {
            return;
        }

        labelSet.resize(graph.getLabelCount());

        for (auto label : labels) {
            labelSet[label] = true;
        }
    }
};

typedef std::vector<ReachQuery> ReachQuerySet;
typedef std::vector<LCRQuery> LCRQuerySet;

std::ostream &operator <<(std::ostream &out, const ReachQuerySet &queries);
std::ostream &operator <<(std::ostream &out, const LCRQuerySet &queries);

std::ostream &operator <<(std::ostream &out, const ReachQuery &query);
std::ostream &operator <<(std::ostream &out, const LCRQuery &query);

std::ostream &printLabels(std::ostream &out, const LabelSet &labelSet);