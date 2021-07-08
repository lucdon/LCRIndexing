#include "PPLIndex.hpp"

bool PPLIndex::isReachable(Vertex sourceComponent, Vertex targetComponent) {
    // Start by doing Pruned Path Labeling.
    auto &outgoingPaths = reachToPath[sourceComponent];
    auto &incomingPaths = reachFromPath[sourceComponent];

    auto outgoingPathCount = outgoingPaths.size();
    auto incomingPathCount = incomingPaths.size();

    size_t outgoingIndex = 0;
    size_t incomingIndex = 0;

    while (outgoingIndex < outgoingPathCount && incomingIndex < incomingPathCount) {
        auto outgoingPath = outgoingPaths[outgoingIndex];
        auto incomingPath = incomingPaths[incomingIndex];

        auto outgoingPathLabel = outgoingPath.first;
        auto incomingPathLabel = incomingPath.first;

        // target is reachable from source if and only if source and target share a label.
        // and the path formed defined as s -> u -> v -> t, has u <= v.
        if (outgoingPathLabel == incomingPathLabel && outgoingPath.second <= incomingPath.second) {
            return true;
        }

        // Values in reachToPath/reachFromPath are sorted, so increase the index with lowest value.
        if (outgoingPathLabel <= incomingPathLabel) {
            outgoingIndex++;
        } else {
            incomingIndex++;
        }
    }

    // If we did not yet find a matching path, then fallback to Pruned Landmark Labeling.
    auto &outgoingLabels = reachTo[sourceComponent];
    auto &incomingLabels = reachFrom[targetComponent];

    auto outgoingLabelCount = outgoingLabels.size();
    auto incomingLabelCount = incomingLabels.size();

    outgoingIndex = 0;
    incomingIndex = 0;

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

static inline size_t calculateDegree(const DiGraph &graph, Vertex vertex) {
    return (graph.getConnected(vertex).size() + 1ull) * (graph.getReverseConnected(vertex).size() + 1ull);
}

void PPLIndex::buildOptimalPath(boost::dynamic_bitset<> &used, std::vector<Vertex> &path) {
    path.clear();
    auto &componentGraph = getGraph();

    std::vector<size_t> vertexScores(componentGraph.getVertexCount(), 0);
    std::vector<size_t> degreeByVertex(componentGraph.getVertexCount());

    for (Vertex vertex = 0u; vertex < componentGraph.getVertexCount(); vertex++) {
        degreeByVertex[vertex] = calculateDegree(componentGraph, vertex);
    }

    Vertex bestVertex = std::numeric_limits<Vertex>::max();
    size_t bestScore = std::numeric_limits<Vertex>::max();

    // Build up score vector.
    for (Vertex vertex = 0u; vertex < componentGraph.getVertexCount(); vertex++) {
        for (auto incoming : componentGraph.getReverseConnected(vertex)) {
            vertexScores[vertex] = std::max(vertexScores[vertex], vertexScores[incoming]);
        }

        if (used[vertex]) {
            degreeByVertex[vertex] = 0ull;
        }

        vertexScores[vertex] += degreeByVertex[vertex];

        if (bestScore < vertexScores[vertex] || bestScore == std::numeric_limits<Vertex>::max()) {
            bestVertex = vertex;
            bestScore = vertexScores[vertex];
        }
    }

    if (bestVertex == std::numeric_limits<Vertex>::max()) {
        return;
    }

    // Back track to construct the path.
    path.push_back(bestVertex);

    while (vertexScores[bestVertex] != degreeByVertex[bestVertex]) {
        auto nextVal = vertexScores[bestVertex] - degreeByVertex[bestVertex];

        for (auto incoming : componentGraph.getReverseConnected(bestVertex)) {
            if (path.size() >= (1ull << 16ull)) {
                break;
            }

            if (vertexScores[incoming] == nextVal) {
                bestVertex = incoming;
                path.push_back(bestVertex);
                break;
            }
        }

        if (path.size() >= (1ull << 16ull)) {
            break;
        }
    }

    // Path has been build in reverse, so reverse to get the actual path.
    std::reverse(path.begin(), path.end());
}

void PPLIndex::prunedBFS(boost::dynamic_bitset<> &visited, boost::dynamic_bitset<> &used, std::queue<Vertex> &queue,
                         std::vector<Vertex> &visitedVertex, std::vector<Vertex> &path, uint32_t pathLabel) {
    auto visitedNum = 0u;
    int pathSize = (int) path.size();

    for (auto pathIndex = pathSize - 1; pathIndex >= 0; pathIndex--) {
        auto vertexOnPath = path[pathIndex];
        queue.push(vertexOnPath);

        // perform bfs from every vertex on the path.
        while (!queue.empty()) {
            auto source = queue.front();
            queue.pop();

            if (visited[source]) {
                // Source is already reachable.
                continue;
            }

            visited[source] = true;
            visitedVertex[visitedNum] = source;
            visitedNum++;

            if (used[source]) {
                // Source is already reachable.
                continue;
            }

            if (this->isReachable(vertexOnPath, source)) {
                // Source is already reachable.
                continue;
            }

            if (pathSize > 1) {
                // Can be indexed as a path.
                this->reachFromPath[source].emplace_back(pathLabel, pathIndex);
            } else {
                // No path index available, so fallback to pruned landmark labeling.
                this->reachFrom[source].push_back(pathLabel);
            }

            for (auto vertex : getGraph().getConnected(source)) {
                queue.push(vertex);
            }
        }
    }

    // Reset visited state for the next round.
    for (int j = 0; j < visitedNum; j++) {
        visited[visitedVertex[j]] = false;
    }
}

void PPLIndex::reversePrunedBFS(boost::dynamic_bitset<> &visited, boost::dynamic_bitset<> &used, std::queue<Vertex> &queue,
                                std::vector<Vertex> &visitedVertex, std::vector<Vertex> &path, uint32_t pathLabel) {
    auto visitedNum = 0u;
    uint32_t pathSize = path.size();

    for (auto pathIndex = 0u; pathIndex < pathSize; pathIndex++) {
        auto vertexOnPath = path[pathIndex];
        queue.push(vertexOnPath);

        // perform reverseBfs from every vertex on the path.
        while (!queue.empty()) {
            auto target = queue.front();
            queue.pop();

            if (visited[target]) {
                // Target is already reachable.
                continue;
            }

            visited[target] = true;
            visitedVertex[visitedNum] = target;
            visitedNum++;

            if (used[target]) {
                // Target is already reachable.
                continue;
            }

            if (this->isReachable(target, vertexOnPath)) {
                // Target is already reachable.
                continue;
            }

            if (pathSize > 1) {
                // Can be indexed as a path.
                this->reachToPath[target].emplace_back(pathLabel, pathIndex);
            } else {
                // No path index available, so fallback to pruned landmark labeling.
                this->reachTo[target].push_back(pathLabel);
            }

            for (auto vertex : getGraph().getReverseConnected(target)) {
                queue.push(vertex);
            }
        }

        used[vertexOnPath] = true;
    }

    // Reset visited state for the next round.
    for (int j = 0; j < visitedNum; j++) {
        visited[visitedVertex[j]] = false;
    }
}

void PPLIndex::train() {
    auto &componentGraph = getGraph();
    int optimalPathNumber = 50;
    const uint32_t minimumPathLength = 5;

    reachToPath.resize(componentGraph.getVertexCount());
    reachFromPath.resize(componentGraph.getVertexCount());
    reachTo.resize(componentGraph.getVertexCount());
    reachFrom.resize(componentGraph.getVertexCount());

    boost::dynamic_bitset<> visited(componentGraph.getVertexCount());
    boost::dynamic_bitset<> used(componentGraph.getVertexCount());

    std::vector<Vertex> order;
    vertexOrderByDegree(componentGraph, order);

    std::vector<Vertex> path;
    std::vector<Vertex> visitedVertex(componentGraph.getVertexCount());

    std::queue<Vertex> queue;
    auto vertexIndex = 0u;
    auto pathLabel = 0u;

    for (auto vertex = 0u; vertex < componentGraph.getVertexCount(); vertex++) {
        // Start by building the paths.
        if (optimalPathNumber > 0) {
            buildOptimalPath(used, path);
            optimalPathNumber--;

            if (path.size() < minimumPathLength) {
                // Path is too short, so stop building paths and fallback to vertex based.
                optimalPathNumber = 0;

                while (vertexIndex < componentGraph.getVertexCount() && used[order[vertexIndex]]) {
                    vertexIndex++;
                }

                if (vertexIndex == componentGraph.getVertexCount()) {
                    break;
                }

                path.clear();
                path.emplace_back(order[vertexIndex]);
            }

            if (path.empty()) {
                continue;
            }
        } else {
            while (vertexIndex < componentGraph.getVertexCount() && used[order[vertexIndex]]) {
                vertexIndex++;
            }

            if (vertexIndex == componentGraph.getVertexCount()) {
                break;
            }

            path.clear();
            path.emplace_back(order[vertexIndex]);
        }

        // pruned forward BFS.
        prunedBFS(visited, used, queue, visitedVertex, path, pathLabel);

        // pruned reverse BFS.
        reversePrunedBFS(visited, used, queue, visitedVertex, path, pathLabel);

        pathLabel++;
    }
}

bool PPLIndex::query(const ReachQuery &query) {
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

size_t PPLIndex::indexSize() const {
    size_t size = 0;

    for (auto i = 0u; i < reachTo.size(); i++) {
        size += reachToPath[i].size() * sizeof(std::pair<uint32_t, uint32_t>);
        size += reachFromPath[i].size() * sizeof(std::pair<uint32_t, uint32_t>);
        size += reachTo[i].size() * sizeof(uint32_t);
        size += reachFrom[i].size() * sizeof(uint32_t);
    }

    return size;
}
