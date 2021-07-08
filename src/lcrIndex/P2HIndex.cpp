#include <utility/CategorizedStepTimer.hpp>
#include "P2HIndex.hpp"

namespace lcr {
    void P2HIndex::train() {
        auto &graph = getGraph();

        boost::dynamic_bitset<> visited(graph.getVertexCount());

        if (graph.getLabelCount() <= numMostFrequentLabels) {
            std::vector<Label> labels(graph.getLabelCount());

            for (Label label = 0u; label < graph.getLabelCount(); label++) {
                labels[label] = label;
            }

            buildPrimaryIndex(graph, visited);
        } else {
            std::vector<Label> labelOrder;
            orderLabelsByFrequency(graph, labelOrder);

            LabelSet labelSet(graph.getLabelCount());

            primaryLabelSet.resize(graph.getLabelCount());
            std::fill(primaryLabelSet.begin(), primaryLabelSet.end(), std::numeric_limits<uint32_t>::max());

            for (auto i = 0; i < numMostFrequentLabels; i++) {
                labelSet[labelOrder[i]] = true;
                primaryLabelSet[labelOrder[i]] = i;
            }

            {
                auto primaryGraph = splitGraph(graph, labelSet, primaryLabelSet);

                buildPrimaryIndex(*primaryGraph, visited);
            }

            visited.reset();
            virtualLabelMapping.resize(graph.getLabelCount());

            {
                auto virtualLabelGraph = createVirtualLabelGraph(graph, labelOrder, numMostFrequentLabels / 2,
                                                                 numMostFrequentLabels / 2, virtualLabelMapping);

                buildSecondaryIndex(*virtualLabelGraph, visited);
            }
        }
    }

    void P2HIndex::buildPrimaryIndex(const LabeledEdgeGraph &graph, boost::dynamic_bitset<> &visited) {
        std::vector<Vertex> order;

        // Order the landmark selection by degree.
        {
            vertexOrderByDegree(graph, order);
        }

        FrontierSetComparatorWithOrder comparatorWithOrder(&order);

        FrontierSet current(comparatorWithOrder);
        FrontierSet plusOne(comparatorWithOrder);

        FrontierSet temp(comparatorWithOrder);

        primaryReachIn.resize(graph.getVertexCount());
        primaryReachOut.resize(graph.getVertexCount());

        for (auto k = 0u; k < graph.getVertexCount(); k++) {
            auto vertex = order[k];

            visited[vertex] = true;

            // First perform pruned bfs for outgoingLabels.
            prunedBFS(graph, primaryReachIn, primaryReachOut, current, plusOne, temp, visited, vertex);

            // Then perform reversed bfs for incomingLabels.
            reversePrunedBFS(graph, primaryReachIn, primaryReachOut, current, plusOne, temp, visited, vertex);
        }
    }

    void P2HIndex::buildSecondaryIndex(const LabeledEdgeGraph &graph, boost::dynamic_bitset<> &visited) {
        // Order the landmark selection by degree.
        std::vector<Vertex> order;
        vertexOrderByDegree(graph, order);

        FrontierSetComparatorWithOrder comparatorWithOrder(&order);

        FrontierSet current(comparatorWithOrder);
        FrontierSet plusOne(comparatorWithOrder);

        FrontierSet temp(comparatorWithOrder);

        secondaryReachIn.resize(graph.getVertexCount());
        secondaryReachOut.resize(graph.getVertexCount());

        for (auto k = 0u; k < graph.getVertexCount(); k++) {
            auto vertex = order[k];

            visited[vertex] = true;

            // First perform pruned bfs for outgoingLabels.
            prunedBFS(graph, secondaryReachIn, secondaryReachOut, current, plusOne, temp, visited, vertex);

            // Then perform reversed bfs for incomingLabels.
            reversePrunedBFS(graph, secondaryReachIn, secondaryReachOut, current, plusOne, temp, visited, vertex);
        }
    }

    void P2HIndex::prunedBFS(const LabeledEdgeGraph &graph, TwoHopIndex &reachIn, TwoHopIndex &reachOut,
                             FrontierSet &current, FrontierSet &plusOne, FrontierSet &temp,
                             boost::dynamic_bitset<> &visited, Vertex vertex) {
        current.clear();
        plusOne.clear();
        temp.clear();

        LabelSet labelSet(graph.getLabelCount());
        auto source = std::make_pair<>(vertex, labelSet);
        current.insert(source);

        while (!current.empty() || !plusOne.empty()) {
            while (!current.empty()) {
                temp.clear();

                for (auto &vertexAndLabels : current) {
                    plusOne.emplace(vertexAndLabels.first, vertexAndLabels.second);

                    auto it = graph.getConnected(vertexAndLabels.first);

                    while (it.next()) {
                        auto &edge = *it;

                        if (visited[edge.target]) {
                            continue;
                        }

                        if (!vertexAndLabels.second[edge.label]) {
                            continue;
                        }

                        if (isReachable(vertex, edge.target, vertexAndLabels.second, reachIn, reachOut)) {
                            continue;
                        }

                        if (insertToIndex(reachIn, edge.target, vertex, vertexAndLabels.second)) {
                            temp.emplace(edge.target, vertexAndLabels.second);
                        }
                    }
                }

                current = temp;
            }

            temp.clear();

            for (auto &vertexAndLabels : plusOne) {
                auto it = graph.getConnected(vertexAndLabels.first);

                while (it.next()) {
                    auto &edge = *it;

                    if (visited[edge.target]) {
                        continue;
                    }

                    if (vertexAndLabels.second[edge.label]) {
                        continue;
                    }

                    LabelSet nextLabels(graph.getLabelCount());

                    nextLabels[edge.label] = true;
                    nextLabels |= vertexAndLabels.second;

                    if (isReachable(vertex, edge.target, nextLabels, reachIn, reachOut)) {
                        continue;
                    }

                    if (insertToIndex(reachIn, edge.target, vertex, nextLabels)) {
                        temp.emplace(edge.target, nextLabels);
                    }
                }
            }

            current = temp;
            plusOne.clear();
        }
    }

    void P2HIndex::reversePrunedBFS(const LabeledEdgeGraph &graph, TwoHopIndex &reachIn, TwoHopIndex &reachOut,
                                    FrontierSet &current, FrontierSet &plusOne, FrontierSet &temp,
                                    boost::dynamic_bitset<> &visited, Vertex vertex) {
        current.clear();
        plusOne.clear();
        temp.clear();

        LabelSet labelSet(graph.getLabelCount());
        auto source = std::make_pair<>(vertex, labelSet);
        current.insert(source);

        while (!current.empty() || !plusOne.empty()) {
            while (!current.empty()) {
                temp.clear();

                for (auto &vertexAndLabels : current) {
                    plusOne.emplace(vertexAndLabels.first, vertexAndLabels.second);

                    auto it = graph.getReverseConnected(vertexAndLabels.first);

                    while (it.next()) {
                        auto &edge = *it;

                        if (visited[edge.target]) {
                            continue;
                        }

                        if (!vertexAndLabels.second[edge.label]) {
                            continue;
                        }

                        if (isReachable(edge.target, vertex, vertexAndLabels.second, reachIn, reachOut)) {
                            continue;
                        }

                        if (insertToIndex(reachOut, edge.target, vertex, vertexAndLabels.second)) {
                            temp.emplace(edge.target, vertexAndLabels.second);
                        }
                    }
                }

                current = temp;
            }

            temp.clear();

            for (auto &vertexAndLabels : plusOne) {
                auto it = graph.getReverseConnected(vertexAndLabels.first);

                while (it.next()) {
                    auto &edge = *it;

                    if (visited[edge.target]) {
                        continue;
                    }

                    if (vertexAndLabels.second[edge.label]) {
                        continue;
                    }

                    LabelSet nextLabels(graph.getLabelCount());

                    nextLabels[edge.label] = true;
                    nextLabels |= vertexAndLabels.second;

                    if (isReachable(edge.target, vertex, nextLabels, reachIn, reachOut)) {
                        continue;
                    }

                    if (insertToIndex(reachOut, edge.target, vertex, nextLabels)) {
                        temp.emplace(edge.target, nextLabels);
                    }
                }
            }

            current = temp;
            plusOne.clear();
        }
    }

    bool P2HIndex::insertToIndex(TwoHopIndex &index, Vertex source, Vertex target, const LabelSet &labelSet) {
        if (target == source) {
            return true;
        }

        auto &part = index[source];

        // Insert in sorted order.
        auto pairToInsert = std::make_pair(target, labelSet);
        auto location = std::lower_bound(part.begin(), part.end(), pairToInsert, VertexLabelPairLessComparator());

        for (; location != part.end(); location++) {
            if (location->first < target) {
                continue;
            }

            if (location->first > target) {
                break;
            }

            if (location->second.is_subset_of(labelSet)) {
                return false;
            }
        }

        bool inserted = false;
        int begin = int(std::distance(part.begin(), location));
        int end = int(part.size()) - 1;

        if (begin != end) {
            for (int i = begin; i <= end; i++) {
                auto &current = part[i];

                if (current.first < target) {
                    continue;
                }

                if (current.first > target) {
                    break;
                }

                if (labelSet.is_subset_of(current.second)) {
                    if (inserted) {
                        if (i != end) {
                            std::swap(part[i], part[end]);
                            i--;
                        }

                        end--;
                    } else {
                        inserted = true;
                        current.second = labelSet;
                    }
                }
            }

            if (end != part.size() - 1) {
                part.erase(part.begin() + end + 1, part.end());
            }
        }

        if (!inserted) {
            part.emplace(location, pairToInsert);
        }

        return true;
    }

    bool P2HIndex::query(const LCRQuery &query) {
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

        if (graph.getLabelCount() <= numMostFrequentLabels) {
            return isPrimaryReachable(source, target, labels);
        }

        LabelSet labelSet(numMostFrequentLabels);
        bool fullyIncluded = true;
        bool included = false;

        for (auto label : query.labels) {
            if (primaryLabelSet[label] != std::numeric_limits<uint32_t>::max()) {
                included = true;
                labelSet[primaryLabelSet[label]] = true;
            } else {
                fullyIncluded = false;
            }
        }

        if (fullyIncluded) {
            return isPrimaryReachable(source, target, labelSet);
        }

        if (included && isPrimaryReachable(source, target, labelSet)) {
            return true;
        }

        if (!isSecondaryReachable(source, target, query.labels)) {
            return false;
        }

        return defaultStrategy(query);
    }

    QueryResult P2HIndex::queryOnce(const LCRQuery &query) {
        return isPrimaryReachable(query.source, query.target, query.labelSet) ? QR_Reachable : QR_NotReachable;
    }

    bool P2HIndex::isPrimaryReachable(Vertex source, Vertex target, const LabelSet &labels) {
        return isReachable(source, target, labels, primaryReachIn, primaryReachOut);
    }

    bool P2HIndex::isSecondaryReachable(Vertex source, Vertex target, const std::vector<Label> &labels) {
        LabelSet virtualLabels(numMostFrequentLabels);

        for (auto label : labels) {
            virtualLabels[virtualLabelMapping[label]] = true;
        }

        return isReachable(source, target, virtualLabels, secondaryReachIn, secondaryReachOut);
    }

    bool P2HIndex::isReachable(Vertex source, Vertex target, const LabelSet &labels, const TwoHopIndex &reachIn,
                               const TwoHopIndex &reachOut) {
        auto &outgoing = reachOut[source];
        auto &incoming = reachIn[target];

        if (outgoing.empty() && incoming.empty()) {
            return false;
        }

        if (outgoing.empty()) {
            // go over incoming, check if source can be found.
            for (auto &incomingPair : incoming) {
                if (incomingPair.first == source) {
                    if (incomingPair.second.is_subset_of(labels)) {
                        return true;
                    }
                }

                if (incomingPair.first > source) {
                    break;
                }
            }

            return false;
        }

        if (incoming.empty()) {
            // go over outgoing, check if target can be found.
            for (auto &outgoingPair : outgoing) {
                if (outgoingPair.first == target) {
                    if (outgoingPair.second.is_subset_of(labels)) {
                        return true;
                    }
                }

                if (outgoingPair.first > target) {
                    break;
                }
            }

            return false;
        }

        size_t outgoingIndex = 0;
        size_t incomingIndex = 0;

        uint32_t incomingFirst = incoming[incomingIndex].first;

        for (; outgoingIndex < outgoing.size(); outgoingIndex++) {
            auto &outgoingPair = outgoing[outgoingIndex];

            if (outgoingPair.first == target) {
                if (outgoingPair.second.is_subset_of(labels)) {
                    return true;
                }
            }

            if (outgoingPair.first > target && incomingIndex >= incoming.size()) {
                return false;
            }

            if (outgoingPair.first < incomingFirst) {
                continue;
            }

            if (!outgoingPair.second.is_subset_of(labels)) {
                continue;
            }

            for (; incomingIndex < incoming.size(); incomingIndex++) {
                auto &incomingPair = incoming[incomingIndex];

                if (incomingPair.first == source) {
                    if (incomingPair.second.is_subset_of(labels)) {
                        // If outgoing is subset of labels and incoming is subset of labels
                        // then target is reachable from source.
                        return true;
                    }
                }

                if (outgoingPair.first < incomingPair.first) {
                    // Must increment outgoing.
                    // Until outgoing.first >= incoming.first.
                    break;
                }

                if (outgoingPair.first > incomingPair.first) {
                    // Must increment incoming.
                    // Until outgoing.first == incoming.first.
                    continue;
                }

                // outgoingPair.first == incomingPair.first
                if (incomingPair.second.is_subset_of(labels)) {
                    // If outgoing is subset of labels and incoming is subset of labels
                    // then target is reachable from source.
                    return true;
                }
            }
        }

        // Finish looping over incoming.
        for (; incomingIndex < incoming.size(); incomingIndex++) {
            auto &incomingPair = incoming[incomingIndex];

            if (incomingPair.first == source) {
                if (incomingPair.second.is_subset_of(labels)) {
                    // If outgoing is subset of labels and incoming is subset of labels
                    // then target is reachable from source.
                    return true;
                }
            }

            if (incomingPair.first > source) {
                break;
            }
        }

        return false;
    }

    bool P2HIndex::defaultStrategy(const LCRQuery &query) {
        auto &graph = getGraph();

        auto source = query.source;
        auto target = query.target;
        auto &labels = query.labelSet;

        // Default to BFS.
        boost::dynamic_bitset<> visited(graph.getVertexCount());
        std::deque<Vertex> queue;
        queue.emplace_back(source);

        visited[source] = true;

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

    size_t P2HIndex::indexSize() const {
        size_t size = 0;

        for (auto i = 0u; i < getVertexCount(); i++) {
            for (const auto &reachableSet : primaryReachIn[i]) {
                size += reachableSet.second.capacity() / 8;
                size += sizeof(Vertex);
            }

            for (const auto &reachableSet : primaryReachOut[i]) {
                size += reachableSet.second.capacity() / 8;
                size += sizeof(Vertex);
            }

            if (getLabelCount() <= numMostFrequentLabels) {
                continue;
            }

            for (const auto &reachableSet : secondaryReachIn[i]) {
                size += reachableSet.second.capacity() / 8;
                size += sizeof(Vertex);
            }

            for (const auto &reachableSet : secondaryReachOut[i]) {
                size += reachableSet.second.capacity() / 8;
                size += sizeof(Vertex);
            }

            size += sizeof(Vertex);
        }

        return size;
    }
}