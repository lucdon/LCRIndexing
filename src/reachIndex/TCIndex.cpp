#include "TCIndex.hpp"

void TCIndex::train() {
    auto &componentGraph = getGraph();
    auto vertices = componentGraph.getVertexCount();

    closure.resize(vertices);

    std::vector<Vertex> reachableSet;

    uint64_t count = 0;

    for (auto source = 0u; source < vertices; source++) {
        closure[source].resize(vertices);

        singleSourceReachable(componentGraph, source, reachableSet);

        for (auto reachable : reachableSet) {
            closure[source][reachable] = true;
            count++;
        }

        reachableSet.clear();
    }
}

bool TCIndex::query(const ReachQuery &query) {
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

    return closure[sourceComponent][targetComponent];
}

size_t TCIndex::indexSize() const {
    // Divide by 8 since the compiler can optimize std::vector<bool> by compacting the booleans to bits.
    return closure.size() * closure.size() / 8u;
}
