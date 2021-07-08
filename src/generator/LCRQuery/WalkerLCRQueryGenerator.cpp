#include "WalkerLCRQueryGenerator.hpp"

void WalkerLCRQueryGenerator::generate(uint32_t numQueries, uint32_t labelCount, LCRQuerySet &lcrQuerySet) {
    if (labelCount == 0) {
        std::cerr << "trying to generate with labelCount = 0" << std::fatal;
    }

    if (labelCount > labeledGraph.getLabelCount()) {
        std::cerr << "trying to generate with a higher labelCount then labels in the graph" << std::fatal;
    }

    uint64_t minPathSize = std::numeric_limits<uint64_t>::max();
    uint64_t maxPathSize = 0;
    uint64_t totalCount = 0;
    uint64_t totalPathCount = 0;

    reset(true);

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
            auto outgoingIt = labeledGraph.getConnected(currentForwardPos);

            if (!outgoingIt.isValid()) {
                isForwardEmpty = true;
                forwardWalk = !forwardWalk;

                // Do not reset, only reset if both paths are empty
                continue;
            }

            auto &nextPos = selectNextStrategy(labeledGraph, outgoingIt, true);
            pathStack.emplace_back(nextPos);
            currentForwardPos = nextPos.target;

            LCRQuery lcrQuery { };

            if (emitQueryStrategy(labeledGraph, nextPos, pathStack, lcrQuery, forwardWalk, labelCount)) {
                auto &query = lcrQuerySet.emplace_back(lcrQuery);
                query.init(labeledGraph);

                currentQuery++;
            }
        } else {
            auto incomingIt = labeledGraph.getReverseConnected(currentBackwardPos);

            if (!incomingIt.isValid()) {
                isBackwardEmpty = true;
                forwardWalk = !forwardWalk;

                // Do not reset, only reset if both paths are empty
                continue;
            }

            auto &nextPos = selectNextStrategy(labeledGraph, incomingIt, false);
            pathStack.emplace_front(nextPos);
            currentBackwardPos = nextPos.target;

            LCRQuery lcrQuery { };

            if (emitQueryStrategy(labeledGraph, nextPos, pathStack, lcrQuery, forwardWalk, labelCount)) {
                auto &query = lcrQuerySet.emplace_back(lcrQuery);
                query.init(labeledGraph);

                currentQuery++;
            }
        }

        if (shouldResetStrategy(labeledGraph, pathStack)) {
            totalCount++;
            minPathSize = std::min(minPathSize, pathStack.size());
            maxPathSize = std::max(maxPathSize, pathStack.size());
            totalPathCount += pathStack.size();

            reset(false);
            continue;
        }

        forwardWalk = !forwardWalk;
    }

    //std::cout << "min path size on reset: " << minPathSize << "\n";
    //std::cout << "avg path size on reset: " << (double(totalPathCount) / double(totalCount)) << "\n";
    //std::cout << "max path size on reset: " << maxPathSize << "\n";
}

// Random state
static std::random_device randomDevice;
static std::default_random_engine randomEngine(randomDevice());

void WalkerLCRQueryGenerator::reset(bool firstTime) {
    pathStack.clear();
    currentForwardPos = 0;
    currentBackwardPos = 0;
    forwardWalk = true;
    isForwardEmpty = false;
    isBackwardEmpty = false;

    for (auto &subscriber : onResetSubscribers) {
        subscriber(labeledGraph, firstTime);
    }

    currentForwardPos = placementStrategy(labeledGraph);
    currentBackwardPos = currentForwardPos;
}

Vertex WalkerLCRQueryGenerator::defaultPlacementStrategy(const LabeledEdgeGraph &labeledGraph) {
    std::uniform_int_distribution<Vertex> distribution(0, labeledGraph.getVertexCount() - 1);
    return distribution(randomEngine);
}

const Edge &
WalkerLCRQueryGenerator::defaultSelectNextStrategy(const LabeledEdgeGraph &labeledGraph, const LabeledEdgeGraphIterator &next,
                                                   bool isForward) {
    std::uniform_int_distribution<Vertex> distribution(0, next.size() - 1);
    return next[distribution(randomEngine)];
}

bool WalkerLCRQueryGenerator::defaultEmitQueryStrategy(const LabeledEdgeGraph &labeledGraph, const Edge &vertex,
                                                       const std::deque<Edge> &pathStack, LCRQuery &outQuery,
                                                       bool isForward, uint32_t labelCount) {
    if (pathStack.empty()) {
        return false;
    }

    std::bernoulli_distribution emitDistribution(0.5);

    if (!emitDistribution(randomEngine)) {
        return false;
    }

    std::uniform_int_distribution<Vertex> distribution(0, pathStack.size() - 1);

    uint32_t idx = distribution(randomEngine);

    if (isForward) {
        outQuery.source = pathStack[idx].source;
        outQuery.target = vertex.target;
    } else {
        outQuery.source = vertex.target;
        outQuery.target = pathStack[idx].source;
    }

    if (outQuery.source == outQuery.target) {
        return false;
    }

    // Compute label set.

    outQuery.labelSet.resize(labeledGraph.getLabelCount());

    uint32_t selectedLabels = 0;

    uint32_t vertexPointer = 0;
    uint32_t pathStackPointer = 0;
    bool selectFromVertex = true;

    while (selectedLabels < labelCount) {
        if (vertexPointer >= outQuery.labelSet.size() && pathStackPointer >= outQuery.labelSet.size()) {
            break;
        }

        if (selectFromVertex) {
            if (vertexPointer >= outQuery.labelSet.size()) {
                selectFromVertex = false;
                continue;
            }

            if (outQuery.labelSet[vertexPointer]) {
                vertexPointer++;
                continue;
            }

            if (vertex.label != vertexPointer) {
                vertexPointer++;
                continue;
            }

            outQuery.labelSet[vertexPointer] = true;
            selectedLabels++;
            vertexPointer++;
            selectFromVertex = false;
        } else {
            if (pathStackPointer >= outQuery.labelSet.size()) {
                selectFromVertex = true;
                continue;
            }

            if (outQuery.labelSet[pathStackPointer]) {
                pathStackPointer++;
                continue;
            }

            if (pathStack[idx].label == pathStackPointer) {
                pathStackPointer++;
                continue;
            }

            outQuery.labelSet[pathStackPointer] = true;
            selectedLabels++;
            pathStackPointer++;
            selectFromVertex = true;
        }
    }

    std::uniform_int_distribution<Label> labelDistribution(0, labeledGraph.getLabelCount() - 1);

    while (selectedLabels < labelCount) {
        auto label = labelDistribution(randomEngine);

        if (!outQuery.labelSet[label]) {
            outQuery.labelSet[label] = true;
            selectedLabels++;
        }
    }

    return true;
}

bool WalkerLCRQueryGenerator::defaultShouldResetStrategy(const LabeledEdgeGraph &labeledGraph,
                                                         const std::deque<Edge> &pathStack) {
    //if (pathStack.size() > 500) {
    //    return true;
    //}

    std::uniform_int_distribution<Vertex> distribution(0, 100);
    return distribution(randomEngine) > 98;
}
