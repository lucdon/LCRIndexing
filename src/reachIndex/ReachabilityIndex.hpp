#pragma once

#include <graphs/Query.hpp>

class ReachabilityIndex {
private:
    SCCGraph* sccGraph = nullptr;

protected:
    bool requiresComponentGraphDuringQueries = true;

public:
    ReachabilityIndex() = default;
    virtual ~ReachabilityIndex() = default;

    ReachabilityIndex(const ReachabilityIndex &) = delete;
    ReachabilityIndex(ReachabilityIndex &&) = default;

    ReachabilityIndex &operator =(const ReachabilityIndex &) = delete;
    ReachabilityIndex &operator =(ReachabilityIndex &&) = default;

    virtual void train() = 0;
    virtual bool query(const ReachQuery &query) = 0;

    /**
     * @brief true -> possibly reachable.
     *       false -> definitely not reachable.
     */
    virtual bool queryOnce(const ReachQuery& reachQuery) {
        // As a default, just fallback to normal querying.
        return query(reachQuery);
    }

    [[nodiscard]] virtual size_t indexSize() const = 0;
    [[nodiscard]] virtual const std::string &getName() const = 0;
    [[nodiscard]] bool canDiscardComponentGraph() const { return !requiresComponentGraphDuringQueries; }

    void setGraph(SCCGraph* graphPtr) {
        this->sccGraph = graphPtr;
    }

    [[nodiscard]] const SCCGraph &getSCCGraph() const {
        return *sccGraph;
    }

    [[nodiscard]] const DiGraph &getGraph() const {
        return sccGraph->getComponentGraph();
    }

    static std::unique_ptr<ReachabilityIndex> create(const std::string &name, std::vector<std::string> &params);

    template<class ... TArgs>
    static std::unique_ptr<ReachabilityIndex> create(const std::string &name, TArgs &&... args) {
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

std::ostream &operator <<(std::ostream &out, const ReachabilityIndex &index);