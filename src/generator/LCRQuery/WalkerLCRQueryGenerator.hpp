#pragma once

#include "graphs/Query.hpp"

/**
 * @brief Random walker only generates true queries.
 * It directly walks the scc graph, so it assumes no loops
 */
class WalkerLCRQueryGenerator {
private:
    const LabeledEdgeGraph &labeledGraph;

private:
    // Strategies
    std::function<Vertex(const LabeledEdgeGraph &)> placementStrategy;
    std::function<const Edge &(const LabeledEdgeGraph &, const LabeledEdgeGraphIterator &, bool)> selectNextStrategy;
    std::function<bool(const LabeledEdgeGraph &, const Edge &, const std::deque<Edge> &, LCRQuery &, bool,
                       uint32_t)> emitQueryStrategy;

    std::function<bool(const LabeledEdgeGraph &, const std::deque<Edge> &)> shouldResetStrategy;

    std::vector<std::function<void(const LabeledEdgeGraph &, bool firstTime)>> onResetSubscribers;

private:
    // Walker state
    uint32_t currentQuery = 0;

    Vertex currentForwardPos = 0;
    Vertex currentBackwardPos = 0;

    bool forwardWalk = true;

    bool isForwardEmpty = false;
    bool isBackwardEmpty = false;

    std::deque<Edge> pathStack;

public:
    explicit WalkerLCRQueryGenerator(const LabeledEdgeGraph &labeledGraph) : labeledGraph(labeledGraph),
                                                                         placementStrategy(defaultPlacementStrategy),
                                                                         selectNextStrategy(defaultSelectNextStrategy),
                                                                         emitQueryStrategy(defaultEmitQueryStrategy),
                                                                         shouldResetStrategy(
                                                                                 defaultShouldResetStrategy) { }

    void setPlacementStrategy(std::function<Vertex(const LabeledEdgeGraph &)> strategy) {
        placementStrategy = std::move(strategy);
    }

    void setSelectNextStrategy(
            std::function<const Edge &(const LabeledEdgeGraph &, const LabeledEdgeGraphIterator &, bool)> strategy) {
        selectNextStrategy = std::move(strategy);
    }

    void setEmitQueryStrategy(
            std::function<bool(const LabeledEdgeGraph &, const Edge &, const std::deque<Edge> &, LCRQuery &,
                               bool, uint32_t)> strategy) {
        emitQueryStrategy = std::move(strategy);
    }

    void setShouldResetStrategy(std::function<bool(const LabeledEdgeGraph &, const std::deque<Edge> &)> strategy) {
        shouldResetStrategy = std::move(strategy);
    }

    void subscribeOnReset(const std::function<void(const LabeledEdgeGraph &, bool firstTime)> &listener) {
        onResetSubscribers.push_back(listener);
    }

    void generate(uint32_t numQueries, uint32_t labelCount, LCRQuerySet &queriesToAddTo);

private:
    /**
     * @brief Resets the walker to start at a random location.
     */
    void reset(bool firstTime);

    /**
     * @brief Default strategy randomly selects a vertex in the component graph.
     */
    static Vertex defaultPlacementStrategy(const LabeledEdgeGraph &labeledGraph);

    static bool defaultShouldResetStrategy(const LabeledEdgeGraph &labeledGraph, const std::deque<Edge> &pathStack);

    static const Edge &
    defaultSelectNextStrategy(const LabeledEdgeGraph &labeledGraph, const LabeledEdgeGraphIterator &nextVertices, bool isForward);

    static bool defaultEmitQueryStrategy(const LabeledEdgeGraph &labeledGraph, const Edge &visitingVertex,
                                         const std::deque<Edge> &pathStack, LCRQuery &outQuery, bool isForward,
                                         uint32_t labelCount);
};
