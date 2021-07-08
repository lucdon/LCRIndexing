#pragma once

#include <dataStructures/BloomFilter.hpp>
#include <graphs/Query.hpp>

namespace lcr {
    struct VertexOriginEntry {
        Vertex vertex;
        Label label;

        VertexOriginEntry(Vertex vertex, Label label) : vertex(vertex), label(label) { }
    };

    typedef std::queue<VertexOriginEntry, std::deque<VertexOriginEntry>> VertexOriginQueue;

    typedef std::pair<Vertex, LabelSet> VertexLabelSet;
    typedef std::unordered_set<VertexLabelSet, boost::hash<VertexLabelSet>> VertexLabelSetVisitedSet;
    typedef std::vector<std::vector<LabelSet>> VisitedMap;

    typedef BloomFilter1Hash BloomFilterUsed;

    struct VertexLabelFreqEntry {
        Vertex vertex;
        LabelSet labelSet;
        std::vector<uint32_t> labelFrequencies;
        int distance;

        VertexLabelFreqEntry(Vertex vertex, size_t labelSize) : vertex(vertex), labelFrequencies(labelSize),
                                                                labelSet(labelSize), distance(0) {

        }

        VertexLabelFreqEntry(Vertex vertex, std::vector<uint32_t> curLabelFrequencies, LabelSet curLabelSet,
                             int curDistance, Label nextLabel) : vertex(vertex),
                                                                 labelFrequencies(std::move(curLabelFrequencies)),
                                                                 labelSet(std::move(curLabelSet)),
                                                                 distance(curDistance) {
            labelFrequencies[nextLabel]++;

            if (!labelSet[nextLabel]) {
                labelSet[nextLabel] = true;
                distance++;
            }
        }
    };

    struct VertexLabelFreqEntryComparator {
        constexpr bool operator ()(VertexLabelFreqEntry const &left, VertexLabelFreqEntry const &right) const {
            return left.distance > right.distance;
        }
    };

    typedef std::priority_queue<VertexLabelFreqEntry, std::deque<VertexLabelFreqEntry>, VertexLabelFreqEntryComparator> VertexLabelFreqQueue;

    struct VertexEntry {
        Vertex vertex;
        LabelSet labelSet;
        int distance;

        VertexEntry(Vertex vertex, LabelSet labelSet, int distance) : vertex(vertex), labelSet(std::move(labelSet)),
                                                                      distance(distance) { }

        VertexEntry(Vertex vertex, size_t labelSetSize, int distance) : vertex(vertex), labelSet(labelSetSize),
                                                                        distance(distance) { }

        VertexEntry(const VertexEntry &entry) {
            vertex = entry.vertex;
            labelSet = entry.labelSet;
            distance = entry.distance;
        }

        VertexEntry(VertexEntry &&entry) noexcept {
            vertex = entry.vertex;
            labelSet = std::move(entry.labelSet);
            distance = entry.distance;
        }

        VertexEntry &operator =(const VertexEntry &entry) = default;;

        VertexEntry &operator =(VertexEntry &&entry) noexcept {
            vertex = entry.vertex;
            labelSet = std::move(entry.labelSet);
            distance = entry.distance;
            return *this;
        };
    };

    struct VertexEntryComparator {
        constexpr bool operator ()(VertexEntry const &left, VertexEntry const &right) const {
            return left.distance > right.distance;
        }
    };

    typedef std::priority_queue<VertexEntry, std::deque<VertexEntry>, VertexEntryComparator> VertexQueue;
    typedef std::priority_queue<VertexEntry, std::deque<VertexEntry>, VertexEntryComparator> VertexQueueVec;

    struct VertexReachEntry {
    public:
        Vertex vertex;
        LabelSet labelSet;
        int distance;
        int reachableIdx;

        VertexReachEntry(Vertex vertex, LabelSet labelSet, int distance, int reachableIdx) : vertex(vertex), labelSet(
                std::move(labelSet)), distance(distance), reachableIdx(reachableIdx) {
        }

        VertexReachEntry(Vertex vertex, size_t labelSetSize, int distance, int reachableIdx) : vertex(vertex),
                                                                                               labelSet(labelSetSize),
                                                                                               distance(distance),
                                                                                               reachableIdx(
                                                                                                       reachableIdx) {
        }

        VertexReachEntry(const VertexReachEntry &entry) {
            vertex = entry.vertex;
            labelSet = entry.labelSet;
            distance = entry.distance;
            reachableIdx = entry.reachableIdx;
        }

        VertexReachEntry(VertexReachEntry &&entry) noexcept {
            vertex = entry.vertex;
            labelSet = std::move(entry.labelSet);
            distance = entry.distance;
            reachableIdx = entry.reachableIdx;
        }

        VertexReachEntry &operator =(const VertexReachEntry &entry) = default;;

        VertexReachEntry &operator =(VertexReachEntry &&entry) noexcept {
            vertex = entry.vertex;
            labelSet = std::move(entry.labelSet);
            distance = entry.distance;
            reachableIdx = entry.reachableIdx;
            return *this;
        };
    };

    struct VertexReachEntryComparator {
        constexpr bool operator ()(VertexReachEntry const &left, VertexReachEntry const &right) const {
            return left.distance > right.distance;
        }
    };

    typedef std::priority_queue<VertexReachEntry, std::deque<VertexReachEntry>, VertexReachEntryComparator> VertexReachQueue;

    struct ReachableEntry {
        LabelSet labelSet;
        boost::dynamic_bitset<> reachable;

        ReachableEntry(LabelSet labelSet, size_t vertexCount) : labelSet(std::move(labelSet)),
                                                                reachable(vertexCount) { }
    };

    struct VertexLabelPairLessComparator {
        constexpr bool operator ()(const std::pair<Vertex, LabelSet> &left, const std::pair<Vertex, LabelSet> &right) {
            return left.first < right.first;
        }
    };

    struct FrontierSetComparatorWithOrder {
        const std::vector<Vertex> *order;

        [[nodiscard]] const std::vector<Vertex> &getOrder() const { return *order; }

        explicit FrontierSetComparatorWithOrder(const std::vector<Vertex> *order) : order(order) { }

        bool operator ()(std::pair<Vertex, LabelSet> const &left, std::pair<Vertex, LabelSet> const &right) const {
            if (left.second.count() == right.second.count()) {
                if (getOrder()[left.first] < getOrder()[right.first]) {
                    return true;
                }

                if (getOrder()[left.first] > getOrder()[right.first]) {
                    return false;
                }

                return left.second < right.second;
            }

            return left.second.count() < right.second.count();
        }
    };

    enum QueryResult {
        QR_Reachable, QR_NotReachable, QR_MaybeReachable
    };

    class Index {
    private:
        LabeledEdgeGraph *labeledGraph = nullptr;

        uint32_t labelCount;
        uint32_t vertexCount;

    public:
        Index() = default;
        virtual ~Index() = default;

        Index(const Index &) = delete;
        Index(Index &&) = default;

        Index &operator =(const Index &) = delete;
        Index &operator =(Index &&) = default;

        virtual void train() = 0;
        virtual bool query(const LCRQuery &query) = 0;

        virtual QueryResult queryOnce(const LCRQuery &q) {
            return query(q) ? QueryResult::QR_Reachable : QueryResult::QR_NotReachable;
        }

        virtual QueryResult queryOnceRecursive(const LCRQuery &q) {
            return queryOnce(q);
        }

        [[nodiscard]] virtual size_t indexSize() const = 0;
        [[nodiscard]] virtual const std::string &getName() const = 0;

        virtual void setGraph(LabeledEdgeGraph *graph) {
            this->labeledGraph = graph;

            // In ScaleHarness the graph is not persisted after training.
            // So store directly in the index.
            labelCount = this->labeledGraph->getLabelCount();
            vertexCount = this->labeledGraph->getVertexCount();
        }

        [[nodiscard]] const LabeledEdgeGraph &getGraph() const {
            return *labeledGraph;
        }

        [[nodiscard]] uint32_t getLabelCount() const {
            return labelCount;
        }

        [[nodiscard]] uint32_t getVertexCount() const {
            return vertexCount;
        }

        static std::unique_ptr<Index> create(const std::string &name, std::vector<std::string> &params);

        template<class ... TArgs>
        static std::unique_ptr<Index> create(const std::string &name, TArgs &&... args) {
            std::vector<std::string> vector;
            addToStringVec(vector, args...);
            return create(name, vector);
        }

    private:
        template<typename TValue, class... TArgs>
        static void addToStringVec(std::vector<std::string> &params, TValue &&value, TArgs &&... args) {
            params.emplace_back(value);
            addToStringVec(params, args...);
        }

        static void addToStringVec(std::vector<std::string> &params) {
        }
    };

    std::ostream &operator <<(std::ostream &out, const Index &index);
}