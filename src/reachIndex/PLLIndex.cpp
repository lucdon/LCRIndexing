#include "PLLIndex.hpp"

bool PLLIndex::isReachable(Vertex sourceComponent, Vertex targetComponent) {
    auto &outgoingLabels = reachTo[sourceComponent];
    auto &incomingLabels = reachFrom[targetComponent];

    auto outgoingLabelCount = outgoingLabels.size();
    auto incomingLabelCount = incomingLabels.size();

    size_t outgoingIndex = 0;
    size_t incomingIndex = 0;

    while (outgoingIndex < outgoingLabelCount && incomingIndex < incomingLabelCount) {
        auto outgoingLabel = outgoingLabels[outgoingIndex];
        auto incomingLabel = incomingLabels[incomingIndex];

        // source and target are reachable if and only if they share a landmark.
        if (outgoingLabel == incomingLabel) {
            return true;
        }

        // Values in incomingLabels/outgoingLabels are sorted, so increase the index with lowest value.
        if (outgoingLabel <= incomingLabel) {
            outgoingIndex++;
        } else {
            incomingIndex++;
        }
    }

    return false;
}

void PLLIndex::prunedBFS(boost::dynamic_bitset<> &visited, std::vector<Vertex> &queue, uint32_t label, Vertex landmark) {
    auto queueStartPtr = 0u;
    auto queueEndPtr = 0u;

    // Enqueue the landmark
    queue[queueEndPtr++] = landmark;

    while (queueStartPtr != queueEndPtr) {
        auto source = queue[queueStartPtr++];

        if (isReachable(landmark, source)) {
            // Source is already reachable.
            continue;
        }

        reachFrom[source].push_back(label);

        for (auto vertex : getGraph().getConnected(source)) {
            if (!visited[vertex]) {
                visited[vertex] = true;
                queue[queueEndPtr++] = vertex;
            }
        }
    }

    // Reset the visited states.
    for (auto i = 0u; i < queueEndPtr; i++) {
        visited[queue[i]] = false;
    }
}

void PLLIndex::reversePrunedBFS(boost::dynamic_bitset<> &visited, std::vector<Vertex> &queue, uint32_t label, Vertex landmark) {
    auto queueStartPtr = 0u;
    auto queueEndPtr = 0u;

    // Enqueue the landmark
    queue[queueEndPtr++] = landmark;

    while (queueStartPtr != queueEndPtr) {
        auto target = queue[queueStartPtr++];

        if (isReachable(target, landmark)) {
            // Source is already reachable.
            continue;
        }

        reachTo[target].push_back(label);

        for (auto vertex : getGraph().getReverseConnected(target)) {
            if (!visited[vertex]) {
                visited[vertex] = true;
                queue[queueEndPtr++] = vertex;
            }
        }
    }

    // Reset the visited states.
    for (auto i = 0u; i < queueEndPtr; i++) {
        visited[queue[i]] = false;
    }
}

void PLLIndex::train() {
    auto& componentGraph = getGraph();
    reachTo.resize(componentGraph.getVertexCount());
    reachFrom.resize(componentGraph.getVertexCount());

    boost::dynamic_bitset<> visited(componentGraph.getVertexCount());
    std::vector<Vertex> queue(componentGraph.getVertexCount());
    std::vector<Vertex> order;

    // Order the landmark selection by degree.
    vertexOrderByDegree(componentGraph, order);

    for (auto vertex = 0u; vertex < componentGraph.getVertexCount(); vertex++) {
        auto landmark = order[vertex];

        // First perform pruned bfs for outgoingLabels.
        prunedBFS(visited, queue, vertex, landmark);

        // Then perform reversed bfs for incomingLabels.
        reversePrunedBFS(visited, queue, vertex, landmark);

        // Mark the vertex as visited.
        visited[landmark] = true;
    }
}

bool PLLIndex::query(const ReachQuery &query) {
    auto& sccGraph = getSCCGraph();

    auto sourceComponent = sccGraph.getComponentIndex(query.source);
    auto targetComponent = sccGraph.getComponentIndex(query.target);

    if (sourceComponent == targetComponent) {
        return true;
    }

    // Base case, since we are querying the strongly connected graph which is a topological sorted DAG.
    if (sourceComponent < targetComponent) {
        return false;
    }

    return this->isReachable(sourceComponent, targetComponent);
}

size_t PLLIndex::indexSize() const {
    size_t size = 0;

    for (auto i = 0u; i < reachTo.size(); i++) {
        size += reachTo[i].size() * sizeof(uint32_t);
        size += reachFrom[i].size() * sizeof(uint32_t);
    }

    return size;
}
