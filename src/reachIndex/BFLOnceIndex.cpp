#include "BFLOnceIndex.hpp"

bool BFLOnceIndex::isReachableOnce(Vertex source, Vertex target) {
    auto sourceInterval = intervalLabels[source];
    auto targetInterval = intervalLabels[target];

    if (sourceInterval.second < targetInterval.second) {
        return false;
    }

    if (sourceInterval.first <= targetInterval.first) {
        return true;
    }

    auto &sourceLabelIn = incomingLabels[source];
    auto &sourceLabelOut = outgoingLabels[source];

    auto &targetLabelIn = incomingLabels[target];
    auto &targetLabelOut = outgoingLabels[target];

    // If nothing leads to target, then it is not reachable.
    if (inEmpty[target]) {
        return false;
    }

    // If there are no edges going out of source, then it is not reachable.
    if (outEmpty[source]) {
        return false;
    }

    if (outEmpty[target]) {
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

    if (inEmpty[source]) {
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

void BFLOnceIndex::reverseDFS(std::vector<uint32_t> &visited, Vertex target, uint32_t curVisited) {
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
            reverseDFS(visited, source, curVisited);
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

void
BFLOnceIndex::forwardDFS(std::vector<uint32_t> &visited, Vertex source, uint32_t &intervalMarker, uint32_t curVisited) {
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
            forwardDFS(visited, target, intervalMarker, curVisited);
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

void BFLOnceIndex::train() {
    auto &componentGraph = getGraph();
    maxCounter = componentGraph.getVertexCount() / intervalCount;

    incomingLabels.resize(componentGraph.getVertexCount());
    outgoingLabels.resize(componentGraph.getVertexCount());
    intervalLabels.resize(componentGraph.getVertexCount());

    inEmpty.resize(componentGraph.getVertexCount());
    outEmpty.resize(componentGraph.getVertexCount());

    std::vector<uint32_t> visited(componentGraph.getVertexCount());
    uint32_t curVisited = 1;

    // First pass reverseDFS.
    for (auto vertex = 0u; vertex < componentGraph.getVertexCount(); vertex++) {
        // Start reverseDFS for all leaves.
        if (componentGraph.getConnected(vertex).empty()) {
            outEmpty[vertex] = true;
            reverseDFS(visited, vertex, curVisited);
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
            inEmpty[vertex] = true;
            forwardDFS(visited, vertex, intervalMarker, curVisited);
        }
    }
}

bool BFLOnceIndex::query(const ReachQuery &query) {
    std::cout << "Not allowed to use query on BFL Once index" << std::fatal;
    return false;
}

bool BFLOnceIndex::queryOnce(const ReachQuery &query) {
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

    return isReachableOnce(sourceComponent, targetComponent);
}

size_t BFLOnceIndex::indexSize() const {
    size_t size = 0;

    for (auto i = 0u; i < incomingLabels.size(); i++) {
        size += incomingLabels[i].size() * sizeof(uint32_t);
        size += outgoingLabels[i].size() * sizeof(uint32_t);

        size += 2;

        size += sizeof(std::pair<uint32_t, uint32_t>);
    }

    return size;
}

uint32_t BFLOnceIndex::hashGet() {
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

void BFLOnceIndex::hashReset() {
    currentHash = randomEngine();
    intervalCounter = 0u;
}
