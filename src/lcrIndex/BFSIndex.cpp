#include "BFSIndex.hpp"

namespace lcr {
    bool BFSIndex::query(const LCRQuery &query) {
        auto &graph = getGraph();

        auto source = query.source;
        auto target = query.target;

        if (source == target) {
            return true;
        }

        auto &labels = query.labelSet;

        if (labels.none()) {
            return false;
        }

        boost::dynamic_bitset<> visited(graph.getVertexCount());
        std::deque<Vertex> queue;

        visited[source] = true;
        queue.emplace_back(source);

        while (!queue.empty()) {
            source = queue.front();
            queue.pop_front();

            auto it = graph.getConnected(source);

            while (it.next()) {
                auto &edge = *it;

                if (!query.labelSet[edge.label]) {
                    continue;
                }

                if (edge.target == target) {
                    return true;
                }

                if (!visited[edge.target]) {
                    visited[edge.target] = true;
                    queue.emplace_back(edge.target);
                }
            }
        }

        return false;
    }
}