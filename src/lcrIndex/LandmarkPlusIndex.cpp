#include <utility/utility.hpp>
#include "LandmarkPlusIndex.hpp"

namespace lcr {
    void LandmarkPlusIndex::train() {
        auto &graph = getGraph();

        std::vector<Vertex> order;
        vertexOrderByDegree(graph, order);

        VertexReachQueue queue;

        auto landmarks = std::min((uint32_t) landmarkCount, (uint32_t) graph.getVertexCount());

        landmarkMap.resize(landmarks);
        reachableBy.resize(landmarks);

        if (nonLandmarkCount > 0) {
            nonLandmarkMap.resize(graph.getVertexCount() - landmarks);
        }

        std::vector<std::vector<LabelSet>> vertexLookup(graph.getVertexCount());

        landmarkMapping.resize(graph.getVertexCount());
        std::fill(landmarkMapping.begin(), landmarkMapping.end(), std::numeric_limits<Vertex>::max());

        for (auto i = 0u; i < landmarks; i++) {
            auto vertex = order[i];
            landmarkMapping[vertex] = i;

            createIndexForLandmark(queue, vertex, vertexLookup);

            uint32_t totalCount = 0;

            for (auto &lookup : vertexLookup) {
                totalCount += lookup.size();
            }

            landmarkMap[i].reserve(totalCount);

            for (auto j = 0u; j < vertexLookup.size(); j++) {
                for (auto k = 0u; k < vertexLookup[j].size(); k++) {
                    landmarkMap[i].emplace_back(j, vertexLookup[j][k]);
                }

                vertexLookup[j].clear();
            }
        }

        if (nonLandmarkCount > 0) {
            for (auto i = 0u; i < graph.getVertexCount() - landmarks; i++) {
                auto vertex = order[i + landmarks];
                landmarkMapping[vertex] = -(i + 1);

                createIndexForNonLandmark(queue, vertex, vertexLookup);

                uint32_t totalCount = 0;

                for (auto &lookup : vertexLookup) {
                    totalCount += lookup.size();
                }

                nonLandmarkMap[i].reserve(totalCount);

                for (auto j = 0u; j < vertexLookup.size(); j++) {
                    for (auto k = 0u; k < vertexLookup[j].size(); k++) {
                        nonLandmarkMap[i].emplace_back(j, vertexLookup[j][k]);
                    }

                    vertexLookup[j].clear();
                }
            }
        }
    }

    bool LandmarkPlusIndex::query(const LCRQuery &query) {
        auto &graph = getGraph();

        auto source = query.source;
        auto target = query.target;

        if (source == target) {
            return true;
        }

        auto &labelSet = query.labelSet;

        if (labelSet.none()) {
            return false;
        }

        if (isLandmark(source)) {
            return queryLandmark(source, target, labelSet);
        }

        boost::dynamic_bitset<> visited(getVertexCount());

        if (isNonLandmark(source)) {
            if (queryNonLandmark(source, target, labelSet, visited)) {
                return true;
            }
        }

        std::deque<Vertex> queue;
        queue.emplace_back(source);

        while (!queue.empty()) {
            source = queue.front();
            queue.pop_front();

            if (visited[source]) {
                continue;
            }

            visited[source] = true;

            if (isLandmark(source)) {
                if (queryExtensive(source, target, labelSet, visited)) {
                    return true;
                }

                continue;
            }

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
                    queue.emplace_back(edge.target);
                }
            }
        }

        return false;
    }

    QueryResult LandmarkPlusIndex::queryOnce(const LCRQuery &query) {
        if (isLandmark(query.source)) {
            if (queryLandmark(query.source, query.target, query.labelSet)) {
                return QR_Reachable;
            }

            return QR_NotReachable;
        }

        if (isNonLandmark(query.source)) {
            if (queryNonLandmark(query.source, query.target, query.labelSet)) {
                return QR_Reachable;
            }
        }

        return QR_MaybeReachable;
    }

    QueryResult LandmarkPlusIndex::queryOnceRecursive(const LCRQuery &query) {
        if (isLandmark(query.source)) {
            if (queryLandmark(query.source, query.target, query.labelSet)) {
                return QR_Reachable;
            }

            return QR_NotReachable;
        }

        return QR_MaybeReachable;
    }

    bool LandmarkPlusIndex::queryExtensive(Vertex landmark, Vertex target, const LabelSet &labelSet,
                                           boost::dynamic_bitset<> &visited) {
        if (queryLandmark(landmark, target, labelSet)) {
            return true;
        }

        for (auto &reachableSet : getReachableBy(landmark)) {
            if (reachableSet.labelSet.is_subset_of(labelSet)) {
                visited |= reachableSet.reachable;
                break;
            }
        }

        return false;
    }

    bool LandmarkPlusIndex::queryLandmark(Vertex landmark, Vertex target, const LabelSet &labelSet) {
        auto &landmarkIndex = getLandmark(landmark);

        auto pair = std::make_pair(target, labelSet);
        auto lower = std::lower_bound(landmarkIndex.begin(), landmarkIndex.end(), pair,
                                      VertexLabelPairLessComparator());

        for (; lower < landmarkIndex.end(); lower++) {
            auto &existingLabelSet = *lower;

            if (existingLabelSet.first < target) {
                continue;
            }

            if (existingLabelSet.first > target) {
                break;
            }

            if (existingLabelSet.second.is_subset_of(labelSet)) {
                return true;
            }
        }

        return false;
    }

    bool LandmarkPlusIndex::queryNonLandmark(Vertex vertex, Vertex target, const LabelSet &labelSet,
                                             boost::dynamic_bitset<> &visited) {
        auto &nonLandmarkIndex = getNonLandmark(vertex);

        auto pair = std::make_pair(target, labelSet);
        auto lower = std::lower_bound(nonLandmarkIndex.begin(), nonLandmarkIndex.end(), pair,
                                      VertexLabelPairLessComparator());

        for (; lower < nonLandmarkIndex.end(); lower++) {
            auto &existingLabelSet = *lower;

            if (existingLabelSet.first < target) {
                continue;
            }

            if (existingLabelSet.first > target) {
                break;
            }

            if (!existingLabelSet.second.is_subset_of(labelSet)) {
                continue;
            }

            if (existingLabelSet.first == target) {
                return true;
            }

            if (queryExtensive(existingLabelSet.first, target, labelSet, visited)) {
                return true;
            }
        }

        return false;
    }

    bool LandmarkPlusIndex::queryNonLandmark(Vertex vertex, Vertex target, const LabelSet &labelSet) {
        auto &nonLandmarkIndex = getNonLandmark(vertex);

        auto pair = std::make_pair(target, labelSet);
        auto lower = std::lower_bound(nonLandmarkIndex.begin(), nonLandmarkIndex.end(), pair,
                                      VertexLabelPairLessComparator());

        for (; lower < nonLandmarkIndex.end(); lower++) {
            auto &existingLabelSet = *lower;

            if (existingLabelSet.first < target) {
                continue;
            }

            if (existingLabelSet.first > target) {
                break;
            }

            if (!existingLabelSet.second.is_subset_of(labelSet)) {
                continue;
            }

            if (existingLabelSet.first == target) {
                return true;
            }

            if (queryLandmark(existingLabelSet.first, target, labelSet)) {
                return true;
            }
        }

        return false;
    }

    void LandmarkPlusIndex::createIndexForLandmark(VertexReachQueue &queue, Vertex landmark,
                                                   std::vector<std::vector<LabelSet>> &vertexLookup) {
        auto &graph = getGraph();

        uint32_t totalCount = 0;
        queue.emplace(landmark, graph.getLabelCount(), -1, -1);
        VertexReachEntry current(landmark, graph.getLabelCount(), -1, -1);

        while (!queue.empty()) {
            current = queue.top();
            queue.pop();

            if (current.distance > 0 && current.distance <= maxReachableLabels) {
                tryInsertReachableEntry(landmark, current.vertex, current.labelSet, current.reachableIdx);
            }

            if (!tryInsert(landmark, current.vertex, current.labelSet, vertexLookup, totalCount)) {
                continue;
            }

            if (isLandmark(current.vertex) && current.vertex != landmark) {
                tryInsertLandmark(landmark, current.vertex, current.labelSet, vertexLookup, totalCount);
                continue;
            }

            auto it = graph.getConnected(current.vertex);

            while (it.next()) {
                auto &edge = *it;

                if (edge.target == landmark) {
                    continue;
                }

                LabelSet labelSet(graph.getLabelCount());
                auto reachableIdx = current.reachableIdx;

                labelSet |= current.labelSet;
                labelSet[edge.label] = true;

                // Distance is equal to the number of labels
                auto distance = (int) labelSet.count();

                // Reset reachable idx when the distance changed.
                if (distance != current.distance) {
                    reachableIdx = -1;
                }

                queue.emplace(edge.target, labelSet, distance, reachableIdx);
            }
        }

        mergeAndFixReachable(landmark);
    }

    void LandmarkPlusIndex::createIndexForNonLandmark(VertexReachQueue &queue, Vertex vertex,
                                                      std::vector<std::vector<LabelSet>> &vertexLookup) {
        auto &graph = getGraph();

        uint32_t totalCount = 0;
        boost::dynamic_bitset<> visited(graph.getVertexCount());
        queue.emplace(vertex, graph.getLabelCount(), -1, -1);
        VertexReachEntry current(vertex, graph.getLabelCount(), -1, -1);

        while (!queue.empty() && totalCount < nonLandmarkCount) {
            current = queue.top();
            queue.pop();

            if (visited[current.vertex]) {
                continue;
            }

            visited[current.vertex] = true;

            if (isLandmark(current.vertex) &&
                !tryInsert(current.vertex, current.vertex, current.labelSet, vertexLookup, totalCount)) {
                continue;
            }

            if (totalCount >= nonLandmarkCount) {
                break;
            }

            if (isLandmark(current.vertex) || (isNonLandmark(current.vertex) && current.vertex != vertex)) {
                tryInsertNonLandmark(vertex, current.vertex, current.labelSet, vertexLookup, totalCount);
                continue;
            }

            auto it = graph.getConnected(current.vertex);

            while (it.next()) {
                auto &edge = *it;

                if (edge.target == vertex) {
                    continue;
                }

                LabelSet labelSet(graph.getLabelCount());
                auto reachableIdx = current.reachableIdx;

                labelSet |= current.labelSet;
                labelSet[edge.label] = true;

                // Distance is equal to the number of labels
                auto distance = (int) labelSet.count();

                // Reset reachable idx when the distance changed.
                if (distance != current.distance) {
                    reachableIdx = -1;
                }

                queue.emplace(edge.target, labelSet, distance, reachableIdx);
            }
        }

        // Empty the queue, before exiting.
        while (!queue.empty()) {
            queue.pop();
        }
    }

    void LandmarkPlusIndex::tryInsertLandmark(Vertex landmark, Vertex otherLandmark, const LabelSet &labelSet,
                                              std::vector<std::vector<LabelSet>> &vertexLookup, uint32_t &totalCount) {
        auto &graph = getGraph();

        for (auto &vertexAndLabels : getLandmark(otherLandmark)) {
            if (vertexAndLabels.first == landmark) {
                continue;
            }

            LabelSet nextLabelSet(graph.getLabelCount());

            nextLabelSet |= labelSet;
            nextLabelSet |= vertexAndLabels.second;

            tryInsert(landmark, vertexAndLabels.first, nextLabelSet, vertexLookup, totalCount);
        }
    }

    void LandmarkPlusIndex::tryInsertNonLandmark(Vertex vertex, Vertex landmark, const LabelSet &labelSet,
                                                 std::vector<std::vector<LabelSet>> &vertexLookup,
                                                 uint32_t &totalCount) {
        auto &graph = getGraph();

        if (isLandmark(landmark)) {
            for (auto &vertexAndLabels : getLandmark(landmark)) {
                if (!isLandmark(vertexAndLabels.first)) {
                    continue;
                }

                LabelSet nextLabelSet(graph.getLabelCount());

                nextLabelSet |= labelSet;
                nextLabelSet |= vertexAndLabels.second;

                tryInsert(vertex, vertexAndLabels.first, nextLabelSet, vertexLookup, totalCount);

                if (totalCount >= nonLandmarkCount) {
                    return;
                }
            }

            return;
        }

        for (auto &vertexAndLabels : getNonLandmark(landmark)) {
            if (!isLandmark(vertexAndLabels.first)) {
                continue;
            }

            LabelSet nextLabelSet(graph.getLabelCount());

            nextLabelSet |= labelSet;
            nextLabelSet |= vertexAndLabels.second;

            tryInsert(vertex, vertexAndLabels.first, nextLabelSet, vertexLookup, totalCount);

            if (totalCount >= nonLandmarkCount) {
                return;
            }
        }
    }

    std::vector<std::pair<Vertex, LabelSet>> &LandmarkPlusIndex::getLandmark(Vertex current) {
        auto index = uint32_t(landmarkMapping[current]);
        return landmarkMap[index];
    }

    std::vector<ReachableEntry> &LandmarkPlusIndex::getReachableBy(Vertex current) {
        auto index = uint32_t(landmarkMapping[current]);
        return reachableBy[index];
    }

    std::vector<std::pair<Vertex, LabelSet>> &LandmarkPlusIndex::getNonLandmark(Vertex current) {
        auto index = uint32_t(-(landmarkMapping[current] + 1));
        return nonLandmarkMap[index];
    }

    bool LandmarkPlusIndex::isLandmark(Vertex current) const {
        return landmarkMapping[current] < landmarkCount && landmarkMapping[current] >= 0;
    }

    bool LandmarkPlusIndex::isNonLandmark(Vertex current) const {
        return landmarkMapping[current] < 0;
    }

    bool LandmarkPlusIndex::tryInsert(Vertex landmark, Vertex target, const LabelSet &labelSet,
                                      std::vector<std::vector<LabelSet>> &vertexLookup, uint32_t &totalCount) {
        if (target == landmark) {
            return true;
        }

        auto &index = vertexLookup[target];

        for (auto &existingLabelSet : index) {
            if (existingLabelSet.is_subset_of(labelSet)) {
                return false;
            }
        }

        bool inserted = false;
        totalCount++;

        if (!index.empty()) {
            int end = int(index.size()) - 1;

            for (int i = 0; i <= end; i++) {
                auto &existingLabelSet = index[i];

                if (labelSet.is_subset_of(existingLabelSet)) {
                    if (inserted) {
                        if (i != end) {
                            std::swap(index[i], index[end]);
                            i--;
                        }

                        totalCount--;
                        end--;
                    } else {
                        existingLabelSet = labelSet;
                        totalCount--;
                        inserted = true;
                    }
                }
            }

            if (end != index.size() - 1) {
                index.erase(index.begin() + end + 1, index.end());
            }
        }

        if (!inserted) {
            index.emplace_back(labelSet);
        }

        return true;
    }

    void LandmarkPlusIndex::tryInsertReachableEntry(Vertex landmark, Vertex vertex, const LabelSet &labelSet,
                                                    int &reachableIdx) {
        auto &reachableSet = getReachableBy(landmark);

        if (reachableIdx == -1) {
            reachableIdx = findReachableEntry(landmark, labelSet);
        }

        if (reachableIdx == -1) {
            reachableSet.emplace_back(labelSet, getGraph().getVertexCount());
            reachableIdx = (int) reachableSet.size() - 1;
        }

        if (isLandmark(vertex)) {
            auto otherReachableIdx = findReachableEntry(vertex, labelSet);

            if (otherReachableIdx != -1) {
                reachableSet[reachableIdx].reachable |= getReachableBy(vertex)[otherReachableIdx].reachable;
            }

            return;
        }

        reachableSet[reachableIdx].reachable[vertex] = true;
    }

    int LandmarkPlusIndex::findReachableEntry(Vertex landmark, const LabelSet &labelSet) {
        auto &reachableIndex = getReachableBy(landmark);

        for (int i = 0; i < reachableIndex.size(); i++) {
            auto &reachableSet = reachableIndex[i];

            if (reachableSet.labelSet == labelSet) {
                return i;
            }
        }
        return -1;
    }

    void LandmarkPlusIndex::mergeAndFixReachable(Vertex landmark) {
        auto &reachableSet = getReachableBy(landmark);

        // First we need to merge the entries
        for (uint32_t i = 0; i < getGraph().getLabelCount(); i++) {
            for (auto j = 0u; j < reachableSet.size(); j++) {
                auto &reachableEntry = reachableSet[j];

                for (auto k = j + 1; k < reachableSet.size(); k++) {
                    auto &otherReachableEntry = reachableSet[k];

                    if (reachableEntry.labelSet.is_subset_of(otherReachableEntry.labelSet)) {
                        otherReachableEntry.reachable |= reachableEntry.reachable;
                    }

                    if (otherReachableEntry.labelSet.is_subset_of(reachableEntry.labelSet)) {
                        reachableEntry.reachable |= otherReachableEntry.reachable;
                    }
                }
            }
        }

        std::vector<uint32_t> counts(reachableSet.size());

        for (auto j = 0u; j < reachableSet.size(); j++) {
            counts[j] = reachableSet[j].reachable.count();
        }

        // Remove those entries that not add minPathLength more than any of their subsets
        for (int j = 0; j < reachableSet.size(); j++) {
            auto &firstSet = reachableSet[j].labelSet;
            auto firstCount = counts[j];

            for (int k = 0; k < reachableSet.size(); k++) {
                if (j == k) {
                    continue;
                }

                auto &secondSet = reachableSet[k].labelSet;
                auto secondCount = counts[k];

                if (firstSet.is_subset_of(secondSet)) {
                    if ((secondCount - firstCount) <= minReachLength) {
                        swapAndPop(reachableSet, k);
                        k--;
                    }
                }
            }
        }

        // We wish to only retain those Reachable entries with at most maxReachableLabels and at least minPathLength bits set.
        std::vector<std::pair<LabelSet, uint32_t>> labelSetPairs;
        labelSetPairs.reserve(reachableSet.size());

        struct sort_pred {
            constexpr bool
            operator ()(const std::pair<LabelSet, uint32_t> &left, const std::pair<LabelSet, uint32_t> &right) {
                return left.second > right.second;
            }
        };

        for (int i = 0; i < reachableSet.size(); i++) {
            LabelSet labelSet = reachableSet[i].labelSet;
            auto count = counts[i];

            if (count < minReachLength) {
                swapAndPop(reachableSet, i);
                i--;
                continue;
            }

            labelSetPairs.emplace_back(labelSet, count);
        }

        // sort on the count
        sort(labelSetPairs.begin(), labelSetPairs.end(), sort_pred());

        for (int i = 0; i < labelSetPairs.size(); i++) {
            int otherReachableIdx = findReachableEntry(landmark, labelSetPairs[i].first);

            // swap the two
            std::swap(reachableSet[i], reachableSet[otherReachableIdx]);
        }
    }

    size_t LandmarkPlusIndex::indexSize() const {
        size_t size = 0;

        for (auto vertex = 0u; vertex < getVertexCount(); vertex++) {
            size += sizeof(Vertex);

            auto index = landmarkMapping[vertex];

            if (isLandmark(vertex)) {
                auto newIndex = uint32_t(index);

                for (auto &vertexAndLabels : landmarkMap[newIndex]) {
                    size += vertexAndLabels.second.capacity() / 8;
                    size += sizeof(Vertex);
                }

                for (auto &reachableEntry : reachableBy[newIndex]) {
                    size += reachableEntry.labelSet.capacity() / 8;
                    size += reachableEntry.reachable.capacity() / 8;
                }
            } else if (isNonLandmark(vertex)) {
                auto newIndex = uint32_t(-(index + 1));

                for (auto &vertexAndLabels : nonLandmarkMap[newIndex]) {
                    size += vertexAndLabels.second.capacity() / 8;
                    size += sizeof(Vertex);
                }
            }
        }

        return size;
    }
}