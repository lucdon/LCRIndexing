#include "BFLIndex.hpp"

bool BFLIndex::isReachable(const DiGraph &componentGraph, Vertex source, Vertex target) {
    auto sourceInterval = intervalLabels[source];
    auto targetInterval = intervalLabels[target];

    if (sourceInterval.second < targetInterval.second) {
        return false;
    }

    if (sourceInterval.first <= targetInterval.first) {
        return true;
    }

    auto &sourceEdgesIn = componentGraph.getReverseConnected(source);
    auto &sourceEdgesOut = componentGraph.getConnected(source);

    auto &targetEdgesIn = componentGraph.getReverseConnected(target);
    auto &targetEdgesOut = componentGraph.getConnected(target);

    auto &sourceLabelIn = incomingLabels[source];
    auto &sourceLabelOut = outgoingLabels[source];

    auto &targetLabelIn = incomingLabels[target];
    auto &targetLabelOut = outgoingLabels[target];

    // If nothing leads to target, then it is not reachable.
    if (targetEdgesIn.empty()) {
        return false;
    }

    // If there are no edges going out of source, then it is not reachable.
    if (sourceEdgesOut.empty()) {
        return false;
    }

    if (targetEdgesOut.empty()) {
        // Check if the outgoing target labels are a subset of the outgoing source labels.
        if ((sourceLabelOut[targetLabelOut[0] >> 5] & (1 << (targetLabelOut[0] & 31))) == 0) {
            return false;
        }
    } else {
        for (auto i = 0u; i < labelSize; i++) {
            // Check if the outgoing target labels are a subset of the outgoing source labels.
            if ((sourceLabelOut[i] & targetLabelOut[i]) != targetLabelOut[i]) {
                return false;
            }
        }
    }

    if (sourceEdgesIn.empty()) {
        // Check if the incoming source labels are a subset of the incoming target labels.
        if ((targetLabelIn[sourceLabelIn[0] >> 5] & (1 << (sourceLabelIn[0] & 31))) == 0) {
            return false;
        }
    } else {
        for (auto i = 0u; i < labelSize; i++) {
            // Check if the incoming source labels are a subset of the incoming target labels.
            if ((sourceLabelIn[i] & targetLabelIn[i]) != sourceLabelIn[i]) {
                return false;
            }
        }
    }

    for (auto adjVertex : componentGraph.getConnected(source)) {
        if (visited[adjVertex] == curVisited) {
            continue;
        }

        visited[adjVertex] = curVisited;

        if (isReachable(componentGraph, adjVertex, target)) {
            return true;
        }
    }

    return false;
}

bool BFLIndex::isReachableOnce(const DiGraph &componentGraph, Vertex source, Vertex target) {
    auto sourceInterval = intervalLabels[source];
    auto targetInterval = intervalLabels[target];

    if (sourceInterval.second < targetInterval.second) {
        return false;
    }

    if (sourceInterval.first <= targetInterval.first) {
        return true;
    }

    auto &sourceEdgesIn = componentGraph.getReverseConnected(source);
    auto &targetEdgesOut = componentGraph.getConnected(target);

    auto &sourceLabelIn = incomingLabels[source];
    auto &sourceLabelOut = outgoingLabels[source];

    auto &targetLabelIn = incomingLabels[target];
    auto &targetLabelOut = outgoingLabels[target];

    if (targetEdgesOut.empty()) {
        // Check if the outgoing target labels are a subset of the outgoing source labels.
        if ((sourceLabelOut[targetLabelOut[0] >> 5] & (1 << (targetLabelOut[0] & 31))) == 0) {
            return false;
        }
    } else {
        for (auto i = 0u; i < labelSize; i++) {
            // Check if the outgoing target labels are a subset of the outgoing source labels.
            if ((sourceLabelOut[i] & targetLabelOut[i]) != targetLabelOut[i]) {
                return false;
            }
        }
    }

    if (sourceEdgesIn.empty()) {
        // Check if the incoming source labels are a subset of the incoming target labels.
        if ((targetLabelIn[sourceLabelIn[0] >> 5] & (1 << (sourceLabelIn[0] & 31))) == 0) {
            return false;
        }
    } else {
        for (auto i = 0u; i < labelSize; i++) {
            // Check if the incoming source labels are a subset of the incoming target labels.
            if ((sourceLabelIn[i] & targetLabelIn[i]) != sourceLabelIn[i]) {
                return false;
            }
        }
    }

    return true;
}

void BFLIndex::reverseDFS(Vertex target) {
    // Mark target visited
    visited[target] = curVisited;
    incomingLabels[target].resize(labelSize);

    auto &incomingVertices = getGraph().getReverseConnected(target);

    if (incomingVertices.empty()) {
        // Nothing connected to target
        incomingLabels[target][0] = hashGet() % labelBitSize;
        return;
    }

    for (auto i = 0u; i < labelSize; i++) {
        incomingLabels[target][i] = 0;
    }

    for (auto source : incomingVertices) {
        if (visited[source] != curVisited) {
            reverseDFS(source);
        }

        if (getGraph().getReverseConnected(source).empty()) {

            // Take the union.
            auto hash = incomingLabels[source][0];
            incomingLabels[target][(hash >> 5) % labelSize] |= 1 << (hash & 31);
            continue;
        }

        // Take the union.
        for (auto i = 0u; i < labelSize; i++) {
            incomingLabels[target][i] |= incomingLabels[source][i];
        }
    }

    auto hash = hashGet();
    incomingLabels[target][(hash >> 5) % labelSize] |= 1 << (hash & 31);
}

void BFLIndex::forwardDFS(Vertex source, uint32_t &intervalMarker) {
    // Mark target visited
    visited[source] = curVisited;
    intervalLabels[source].first = intervalMarker++;
    outgoingLabels[source].resize(labelSize);

    auto &outgoingVertices = getGraph().getConnected(source);

    if (outgoingVertices.empty()) {
        // Nothing connected to target
        outgoingLabels[source][0] = hashGet() % labelBitSize;
        intervalLabels[source].second = intervalMarker;
        return;
    }

    for (auto i = 0u; i < labelSize; i++) {
        outgoingLabels[source][i] = 0;
    }

    for (auto target : outgoingVertices) {
        if (visited[target] != curVisited) {
            forwardDFS(target, intervalMarker);
        }

        if (getGraph().getConnected(target).empty()) {

            // Take the union.
            auto hash = outgoingLabels[target][0];
            outgoingLabels[source][(hash >> 5) % labelSize] |= 1 << (hash & 31);
            continue;
        }

        // Take the union.
        for (auto i = 0u; i < labelSize; i++) {
            outgoingLabels[source][i] |= outgoingLabels[target][i];
        }
    }

    auto hash = hashGet();
    outgoingLabels[source][(hash >> 5) % labelSize] |= 1 << (hash & 31);
    intervalLabels[source].second = intervalMarker;
}

void BFLIndex::train() {
    auto &componentGraph = getGraph();
    maxCounter = componentGraph.getVertexCount() / intervalCount;

    incomingLabels.resize(componentGraph.getVertexCount());
    outgoingLabels.resize(componentGraph.getVertexCount());
    intervalLabels.resize(componentGraph.getVertexCount());

    visited.resize(componentGraph.getVertexCount());
    curVisited = 1;

    // First pass reverseDFS.
    for (auto vertex = 0u; vertex < componentGraph.getVertexCount(); vertex++) {
        // Start reverseDFS for all leaves.
        if (componentGraph.getConnected(vertex).empty()) {
            reverseDFS(vertex);
        }
    }

    // Reset visited state.
    curVisited++;
    hashReset();
    uint32_t intervalMarker = 0;

    // Second pass, forwardDFS.
    for (auto vertex = 0u; vertex < componentGraph.getVertexCount(); vertex++) {
        // Start forwardDFS for all roots.
        if (componentGraph.getReverseConnected(vertex).empty()) {
            forwardDFS(vertex, intervalMarker);
        }
    }
}

bool BFLIndex::query(const ReachQuery &query) {
    auto &sccGraph = getSCCGraph();

    auto sourceComponent = sccGraph.getComponentIndex(query.source);
    auto targetComponent = sccGraph.getComponentIndex(query.target);

    if (sourceComponent == targetComponent) {
        return true;
    }

    // Base case, since we are querying the strongly connected graph which is a topological sorted DAG.
    if (sourceComponent < targetComponent) {
        return false;
    }

    curVisited++;
    return isReachable(getGraph(), sourceComponent, targetComponent);
}

bool BFLIndex::queryOnce(const ReachQuery &query) {
    auto &sccGraph = getSCCGraph();

    auto sourceComponent = sccGraph.getComponentIndex(query.source);
    auto targetComponent = sccGraph.getComponentIndex(query.target);

    if (sourceComponent == targetComponent) {
        return true;
    }

    // Base case, since we are querying the strongly connected graph which is a topological sorted DAG.
    if (sourceComponent < targetComponent) {
        return false;
    }

    return isReachableOnce(getGraph(), sourceComponent, targetComponent);
}

size_t BFLIndex::indexSize() const {
    size_t size = 0;

    for (auto i = 0u; i < getGraph().getVertexCount(); i++) {
        size += incomingLabels[i].size() * sizeof(uint32_t);
        size += outgoingLabels[i].size() * sizeof(uint32_t);

        size += sizeof(std::pair<uint32_t, uint32_t>);
    }

    return size;
}

uint32_t BFLIndex::hashGet() {
    if (intervalCounter >= maxCounter) {
        // Stop producing in the current interval.
        // So reset the counter and assign a new interval.
        hashReset();
    }

    // Increase the interval counter.
    intervalCounter++;

    // Return the new hash.
    return currentHash;
}

void BFLIndex::hashReset() {
    currentHash = randomEngine();
    intervalCounter = 0u;
}
