#include "WalkerQueryGenerator.hpp"

std::shared_ptr<ReachQuerySet> WalkerQueryGenerator::generate(uint32_t numQueries) {
    auto reachQuerySetPtr = std::make_shared<ReachQuerySet>(numQueries);
    auto &reachQuerySet = *reachQuerySetPtr;

    uint64_t minPathSize = std::numeric_limits<uint64_t>::max();
    uint64_t maxPathSize = 0;
    uint64_t totalCount = 0;
    uint64_t totalPathCount = 0;

    Vertex nextPos = 0;

    reset(true);
    ReachQuery reachQuery { };

    while (currentQuery < numQueries) {
        if (isBackwardEmpty && isForwardEmpty) {
            totalCount++;
            minPathSize = std::min(minPathSize, pathStack.size());
            maxPathSize = std::max(maxPathSize, pathStack.size());
            totalPathCount += pathStack.size();

            reset(false);
            continue;
        }

        if (forwardWalk) {
            auto &outgoing = sccGraph.getComponentGraph().getConnected(currentForwardPos);

            if (outgoing.empty()) {
                isForwardEmpty = true;
                forwardWalk = !forwardWalk;

                // Do not reset, only reset if both paths are empty
                continue;
            }

            nextPos = selectNextStrategy(sccGraph, outgoing, true);
            pathStack.emplace_back(nextPos);
            currentForwardPos = nextPos;
        } else {
            auto &incoming = sccGraph.getComponentGraph().getReverseConnected(currentBackwardPos);

            if (incoming.empty()) {
                isBackwardEmpty = true;
                forwardWalk = !forwardWalk;

                // Do not reset, only reset if both paths are empty
                continue;
            }

            nextPos = selectNextStrategy(sccGraph, incoming, false);
            pathStack.emplace_front(nextPos);
            currentBackwardPos = nextPos;
        }

        if (emitQueryStrategy(sccGraph, nextPos, pathStack, reachQuery, forwardWalk)) {
            reachQuerySet[currentQuery].source = selectRandomVertex(reachQuery.source);
            reachQuerySet[currentQuery].target = selectRandomVertex(reachQuery.target);

            currentQuery++;
        }

        if (shouldResetStrategy(sccGraph, pathStack)) {
            totalCount++;
            minPathSize = std::min(minPathSize, pathStack.size());
            maxPathSize = std::max(maxPathSize, pathStack.size());
            totalPathCount += pathStack.size();

            reset(false);
            continue;
        }

        forwardWalk = !forwardWalk;
    }

    std::cout << "min path size on reset: " << minPathSize << "\n";
    std::cout << "avg path size on reset: " << (double(totalPathCount) / double(totalCount)) << "\n";
    std::cout << "max path size on reset: " << maxPathSize << "\n";

    return reachQuerySetPtr;
}

// Random state
static std::random_device randomDevice;
static std::default_random_engine randomEngine(randomDevice());

void WalkerQueryGenerator::reset(bool firstTime) {
    pathStack.clear();
    currentForwardPos = 0;
    currentBackwardPos = 0;
    forwardWalk = true;
    isForwardEmpty = false;
    isBackwardEmpty = false;

    for (auto &subscriber : onResetSubscribers) {
        subscriber(sccGraph, firstTime);
    }

    currentForwardPos = placementStrategy(sccGraph);
    currentBackwardPos = currentForwardPos;
    pathStack.emplace_back(currentForwardPos);
}

Vertex WalkerQueryGenerator::defaultPlacementStrategy(const SCCGraph &sccGraph) {
    std::uniform_int_distribution<Vertex> distribution(0, sccGraph.getComponentGraph().getVertexCount() - 1);
    return distribution(randomEngine);
}

Vertex WalkerQueryGenerator::defaultSelectNextStrategy(const SCCGraph &sccGraph, const std::vector<Vertex> &next,
                                                       bool isForward) {
    std::uniform_int_distribution<Vertex> distribution(0, next.size() - 1);
    return next[distribution(randomEngine)];
}

bool WalkerQueryGenerator::defaultEmitQueryStrategy(const SCCGraph &sccGraph, Vertex vertex,
                                                    const std::deque<Vertex> &pathStack, ReachQuery &outQuery,
                                                    bool isForward) {
    std::bernoulli_distribution emitDistribution(0.5);

    if (!emitDistribution(randomEngine)) {
        return false;
    }

    std::uniform_int_distribution<Vertex> distribution(0, pathStack.size() - 1);

    if (isForward) {
        outQuery.source = pathStack[distribution(randomEngine)];
        outQuery.target = vertex;
    } else {
        outQuery.source = vertex;
        outQuery.target = pathStack[distribution(randomEngine)];
    }

    return outQuery.source != outQuery.target;
}

bool WalkerQueryGenerator::defaultShouldResetStrategy(const SCCGraph &sccGraph, const std::deque<Vertex> &pathStack) {
    if (pathStack.size() > 50) {
       // return true;
    }

   // std::uniform_int_distribution<Vertex> distribution(0, 100);
    return false;//distribution(randomEngine) > 98;
}

Vertex WalkerQueryGenerator::selectRandomVertex(Vertex component) {
    auto &vertices = sccGraph.getVerticesForComponent(component);
    std::uniform_int_distribution<Vertex> distribution(0, vertices.size() - 1);
    return vertices[distribution(randomEngine)];
}
