#include "LWBFIndex.hpp"

namespace lcr {
    bool LWBFIndex::isReachable(Vertex source, Vertex target, const LabelSet &labelSet) {
        if (!isBloomFilter(source)) {
            return true;
        }

        if (isBloomFilter(target)) {
            bool isMaybeReachable = false;

            for (auto &targetLabelIn : incomingLabels[bloomFilterMapping[target]]) {
                if (!targetLabelIn.first.is_subset_of(labelSet)) {
                    continue;
                }

                if (targetLabelIn.second.contains(source)) {
                    isMaybeReachable = true;
                    break;
                }
            }

            if (!isMaybeReachable) {
                return false;
            }
        }

        bool isMaybeReachable = false;

        for (auto &sourceLabelOut : outgoingLabels[bloomFilterMapping[source]]) {
            if (!sourceLabelOut.first.is_subset_of(labelSet)) {
                continue;
            }

            if (sourceLabelOut.second.contains(target)) {
                isMaybeReachable = true;
                break;
            }
        }

        return isMaybeReachable;
    }

    void LWBFIndex::train() {
        auto &graph = getGraph();

        std::vector<Vertex> order;
        vertexOrderByDegree(graph, order);

        LWBFTrainState trainState(numBloomFilters, graph.getVertexCount());

        landmarkMapping.resize(graph.getVertexCount());
        bloomFilterMapping.resize(graph.getVertexCount());

        landmarkMap.resize(landmarkCount);
        incomingLabels.resize(numBloomFilters);
        outgoingLabels.resize(numBloomFilters);

        std::fill(landmarkMapping.begin(), landmarkMapping.end(), std::numeric_limits<uint32_t>::max());
        std::fill(bloomFilterMapping.begin(), bloomFilterMapping.end(), std::numeric_limits<uint32_t>::max());

        for (uint32_t i = 0; i < graph.getVertexCount(); i++) {
            auto vertex = order[i];

            if (i < landmarkCount) {
                landmarkMapping[vertex] = i;
            }

            if (i < numBloomFilters) {
                bloomFilterMapping[vertex] = i;
            }
        }

        for (auto i = 0u; i < landmarkCount; i++) {
            auto vertex = order[i];

            forwardBFS(vertex, trainState);

            trainState.landmarkVisited[vertex] = true;
            uint32_t totalCount = 0;

            for (auto &lookup : trainState.vertexLookup) {
                totalCount += lookup.size();
            }

            landmarkMap[i].reserve(totalCount);

            for (auto j = 0u; j < trainState.vertexLookup.size(); j++) {
                for (auto k = 0u; k < trainState.vertexLookup[j].size(); k++) {
                    landmarkMap[i].emplace_back(j, trainState.vertexLookup[j][k]);
                }

                trainState.vertexLookup[j].clear();
            }

            if (i < numBloomFilters) {
                convertLandmarkToBloomFilter(vertex, trainState);
            }
        }

        for (auto i = landmarkCount; i < numBloomFilters; i++) {
            auto vertex = order[i];

            forwardBFS(vertex, trainState);
            createBloomFilter(vertex, trainState);
        }

        for (auto i = 0u; i < numBloomFilters; i++) {
            incomingLabels[i].reserve(trainState.incomingLabels[i].size());

            for (auto &bloomFilter : trainState.incomingLabels[i]) {
                auto &pair = incomingLabels[i].emplace_back();
                pair.first = bloomFilter.first;
                pair.second = std::move(bloomFilter.second);
            }
        }
    }

    void LWBFIndex::forwardBFS(Vertex vertex, LWBFTrainState &trainState) {
        auto &graph = getGraph();

        trainState.queue.emplace(vertex, graph.getLabelCount(), -1);
        VertexEntry current(vertex, graph.getLabelCount(), -1);

        while (!trainState.queue.empty()) {
            current = trainState.queue.top();
            trainState.queue.pop();

            if (!tryInsert(vertex, current.vertex, current.labelSet, trainState)) {
                continue;
            }

            if (trainState.landmarkVisited[current.vertex] && current.vertex != vertex) {
                tryInsertLandmark(vertex, current.vertex, current.labelSet, trainState);
                continue;
            }

            auto it = graph.getConnected(current.vertex);

            while (it.next()) {
                auto &edge = *it;

                if (edge.target == vertex) {
                    continue;
                }

                LabelSet labelSet(graph.getLabelCount());

                labelSet |= current.labelSet;
                labelSet[edge.label] = true;

                // Distance is equal to the number of labels
                auto distance = (int) labelSet.count();

                trainState.queue.emplace(edge.target, labelSet, distance);
            }
        }
    }

    void LWBFIndex::createBloomFilter(Vertex vertex, LWBFTrainState &trainState) {
        std::map<LabelSet, std::vector<Vertex>> labelSetMapping;

        for (auto j = 0u; j < trainState.vertexLookup.size(); j++) {
            for (auto k = 0u; k < trainState.vertexLookup[j].size(); k++) {
                labelSetMapping[trainState.vertexLookup[j][k]].emplace_back(j);
            }

            trainState.vertexLookup[j].clear();
        }

        for (auto &pair : labelSetMapping) {
            auto &bloomFilterOut = outgoingLabels[bloomFilterMapping[vertex]].emplace_back();

            bloomFilterOut.first = pair.first;
            bloomFilterOut.second.setup(bloomFilterBits);

            for (auto reachableVertex : pair.second) {
                bloomFilterOut.second.add(reachableVertex);

                if (isBloomFilter(reachableVertex)) {
                    auto &incomingMap = trainState.incomingLabels[bloomFilterMapping[reachableVertex]];
                    auto &bloomFilterIn = incomingMap[pair.first];

                    if (bloomFilterIn.empty()) {
                        bloomFilterIn.setup(bloomFilterBits);
                    }

                    bloomFilterIn.add(vertex);
                }
            }
        }
    }

    void LWBFIndex::convertLandmarkToBloomFilter(Vertex vertex, LWBFTrainState &trainState) {
        std::map<LabelSet, std::vector<Vertex>> labelSetMapping;

        auto &landmark = landmarkMap[landmarkMapping[vertex]];

        for (auto &pair : landmark) {
            labelSetMapping[pair.second].emplace_back(pair.first);
        }

        for (auto &pair : labelSetMapping) {
            auto &bloomFilterOut = outgoingLabels[bloomFilterMapping[vertex]].emplace_back();

            bloomFilterOut.first = pair.first;
            bloomFilterOut.second.setup(bloomFilterBits);

            for (auto reachableVertex : pair.second) {
                bloomFilterOut.second.add(reachableVertex);

                if (isBloomFilter(reachableVertex)) {
                    auto &incomingMap = trainState.incomingLabels[bloomFilterMapping[reachableVertex]];
                    auto &bloomFilterIn = incomingMap[pair.first];

                    if (bloomFilterIn.empty()) {
                        bloomFilterIn.setup(bloomFilterBits);
                    }

                    bloomFilterIn.add(vertex);
                }
            }
        }
    }

    bool LWBFIndex::tryInsert(Vertex landmark, Vertex target, const LabelSet &labelSet, LWBFTrainState &trainState) {
        if (target == landmark) {
            return true;
        }

        auto &index = trainState.vertexLookup[target];

        for (auto &existingLabelSet : index) {
            if (existingLabelSet.is_subset_of(labelSet)) {
                return false;
            }
        }

        bool inserted = false;

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

                        end--;
                    } else {
                        existingLabelSet = labelSet;
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

    void LWBFIndex::tryInsertLandmark(Vertex landmark, Vertex otherLandmark, const LabelSet &labelSet,
                                      LWBFTrainState &trainState) {
        auto &graph = getGraph();

        for (auto &vertexAndLabels : landmarkMap[landmarkMapping[otherLandmark]]) {
            if (vertexAndLabels.first == landmark) {
                continue;
            }

            LabelSet nextLabelSet(graph.getLabelCount());

            nextLabelSet |= labelSet;
            nextLabelSet |= vertexAndLabels.second;

            tryInsert(landmark, vertexAndLabels.first, nextLabelSet, trainState);
        }
    }

    bool LWBFIndex::query(const LCRQuery &query) {
        auto &graph = getGraph();

        auto source = query.source;
        auto target = query.target;

        if (source == target) {
            return true;
        }

        auto &labels = query.labels;

        if (labels.empty()) {
            return false;
        }

        if (!isReachable(source, target, query.labelSet)) {
            return false;
        }

        if (isLandmark(source)) {
            return queryLandmark(source, target, query.labelSet);
        }

        boost::dynamic_bitset<> visited(graph.getVertexCount());
        std::deque<Vertex> queue;

        visited[source] = true;
        queue.emplace_back(source);

        while (!queue.empty()) {
            source = queue.back();
            queue.pop_back();

            if (!isReachable(source, target, query.labelSet)) {
                continue;
            }

            if (isLandmark(source)) {
                if (queryLandmark(source, target, query.labelSet, visited)) {
                    return true;
                } else {
                    continue;
                }
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
                    visited[edge.target] = true;
                    queue.emplace_back(edge.target);
                }
            }
        }

        return false;
    }

    QueryResult LWBFIndex::queryOnce(const LCRQuery &query) {
        if (!isReachable(query.source, query.target, query.labelSet)) {
            return QR_NotReachable;
        }

        if (isLandmark(query.source)) {
            return queryLandmark(query.source, query.target, query.labelSet) ? QR_Reachable : QR_NotReachable;
        }

        return QR_MaybeReachable;
    }

    QueryResult LWBFIndex::queryOnceRecursive(const LCRQuery &query) {
        if (!isReachable(query.source, query.target, query.labelSet)) {
            return QR_NotReachable;
        }

        if (isLandmark(query.source)) {
            return queryLandmark(query.source, query.target, query.labelSet) ? QR_Reachable : QR_NotReachable;
        }

        return QR_MaybeReachable;
    }

    bool LWBFIndex::queryLandmark(Vertex landmark, Vertex target, const LabelSet &labelSet) {
        auto &landmarkIndex = landmarkMap[landmarkMapping[landmark]];

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

    bool LWBFIndex::queryLandmark(Vertex landmark, Vertex target, const LabelSet &labelSet,
                                  boost::dynamic_bitset<> &visited) {
        auto index = landmarkMapping[landmark];
        auto &landmarkIndex = landmarkMap[index];

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

    size_t LWBFIndex::indexSize() const {
        size_t size = 0;

        for (auto &bloomFilterIndex : outgoingLabels) {
            for (auto &bloomFilter : bloomFilterIndex) {
                size += bloomFilter.first.capacity() / 8;
                size += bloomFilter.second.sizeInBytes();
            }
        }

        for (auto &bloomFilterIndex : incomingLabels) {
            for (auto &bloomFilter : bloomFilterIndex) {
                size += bloomFilter.first.capacity() / 8;
                size += bloomFilter.second.sizeInBytes();
            }
        }

        size += sizeof(Vertex) * landmarkMapping.size();
        size += sizeof(Vertex) * bloomFilterMapping.size();

        for (auto &landmark : landmarkMap) {
            for (auto &target : landmark) {
                size += sizeof(Vertex);
                size += target.second.capacity() / 8;
            }
        }

        return size;
    }

    bool LWBFIndex::isLandmark(Vertex vertex) const {
        return landmarkMapping[vertex] != std::numeric_limits<uint32_t>::max();
    }

    bool LWBFIndex::isBloomFilter(Vertex vertex) const {
        return bloomFilterMapping[vertex] != std::numeric_limits<uint32_t>::max();
    }
}