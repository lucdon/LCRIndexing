#include "BiBFSIndex.hpp"

bool BiBFSIndex::query(const ReachQuery& query) {
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

    return reachabilityBiBFS(getGraph(), sourceComponent, targetComponent);
}
