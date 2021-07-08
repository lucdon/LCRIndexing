#pragma once

#include "graphs/Query.hpp"

/**
 * @brief Random walker only generates true queries.
 * It directly walks the scc graph, so it assumes no loops
 */
class WalkerQueryGenerator {
private:
    const SCCGraph &sccGraph;

private:
    // Strategies
    std::function<Vertex(const SCCGraph &)> placementStrategy;
    std::function<Vertex(const SCCGraph &, const std::vector<Vertex> &, bool)> selectNextStrategy;
    std::function<bool(const SCCGraph &, Vertex, const std::deque<Vertex> &, ReachQuery &, bool)> emitQueryStrategy;
    std::function<bool(const SCCGraph &, const std::deque<Vertex> &)> shouldResetStrategy;

    std::vector<std::function<void(const SCCGraph &, bool firstTime)>> onResetSubscribers;

private:
    // Walker state
    uint32_t currentQuery = 0;

    Vertex currentForwardPos = 0;
    Vertex currentBackwardPos = 0;

    bool forwardWalk = true;

    bool isForwardEmpty = false;
    bool isBackwardEmpty = false;

    std::deque<Vertex> pathStack;

public:
    explicit WalkerQueryGenerator(const SCCGraph &sccGraph) : sccGraph(sccGraph),
                                                              placementStrategy(defaultPlacementStrategy),
                                                              selectNextStrategy(defaultSelectNextStrategy),
                                                              emitQueryStrategy(defaultEmitQueryStrategy),
                                                              shouldResetStrategy(defaultShouldResetStrategy) { }

    void setPlacementStrategy(std::function<Vertex(const SCCGraph &)> strategy) {
        placementStrategy = std::move(strategy);
    }

    void setSelectNextStrategy(std::function<Vertex(const SCCGraph &, const std::vector<Vertex> &, bool)> strategy) {
        selectNextStrategy = std::move(strategy);
    }

    void setEmitQueryStrategy(
            std::function<bool(const SCCGraph &, Vertex, const std::deque<Vertex> &, ReachQuery &, bool)> strategy) {
        emitQueryStrategy = std::move(strategy);
    }

    void setShouldResetStrategy(std::function<bool(const SCCGraph &, const std::deque<Vertex> &)> strategy) {
        shouldResetStrategy = std::move(strategy);
    }

    void subscribeOnReset(const std::function<void(const SCCGraph &, bool firstTime)> &listener) {
        onResetSubscribers.push_back(listener);
    }

    std::shared_ptr<ReachQuerySet> generate(uint32_t numQueries);

private:
    /**
     * @brief Resets the walker to start at a random location.
     */
    void reset(bool firstTime);

    /**
     * @brief Default strategy randomly selects a vertex in the component graph.
     */
    static Vertex defaultPlacementStrategy(const SCCGraph &sccGraph);

    static bool defaultShouldResetStrategy(const SCCGraph &sccGraph, const std::deque<Vertex> &pathStack);

    static Vertex
    defaultSelectNextStrategy(const SCCGraph &sccGraph, const std::vector<Vertex> &nextVertices, bool isForward);

    static bool
    defaultEmitQueryStrategy(const SCCGraph &sccGraph, Vertex visitingVertex, const std::deque<Vertex> &pathStack,
                             ReachQuery &outQuery, bool isForward);

    Vertex selectRandomVertex(Vertex source);
};
