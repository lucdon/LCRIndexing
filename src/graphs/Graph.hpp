#pragma once

#include <algorithms/GraphAlgorithms.hpp>

class Graph {
private:
    std::vector<EdgeList> adj;

    size_t edgeCount = 0;

public:
    Graph() = default;

    Graph(const Graph &) = delete;
    Graph(Graph &&) = default;

    Graph &operator =(const Graph &) = delete;
    Graph &operator =(Graph &&) = default;

    void setVertices(size_t size) {
        adj.resize(size);
    }

    [[nodiscard]] size_t getVertexCount() const {
        return adj.size();
    }

    [[nodiscard]] size_t getEdgeCount() const {
        return edgeCount;
    }

    [[nodiscard]] size_t getSizeInBytes() const {
        size_t size = 0;

        for (const auto &connected : adj) {
            size += connected.size() * sizeof(Vertex);
        }

        return size;
    }

    /**
     * @brief Add (source, target) as edge.
     * It will only add the edge if it has not already been added.
     * It will crash the application if the edge is not within the graphs bounds.
     */
    bool addEdge(Vertex source, Vertex target) {
        if (source >= adj.size() || target >= adj.size()) {
            std::cerr << "Failed adding edge, source or target out of bounds! source: " << source << " target: "
                      << target << " max: " << adj.size() << std::fatal;
        }

        if (edgeExists(source, target)) {
            return false;
        }

        edgeCount++;
        adj[source].push_back(target);
        return true;
    }

    /**
     * @brief Add (source, target) as edge. Does not perform checks.
     * Thus assumes: edge is unique and within bounds.
     */
    void addEdgeNoChecks(Vertex source, Vertex target) {
        edgeCount++;
        adj[source].push_back(target);
    }

    /**
     * @brief Add a list of edges starting from source. Does not perform checks.
     * Thus assumes: all edge are unique and within bounds.
     */
    void addEdgesNoChecks(Vertex source, std::vector<Vertex> &targets) {
        edgeCount += targets.size();

        uint32_t currentEnd = uint32_t(adj[source].size());
        adj[source].reserve(currentEnd + targets.size());
        adj[source].insert(adj[source].begin() + currentEnd, targets.begin(), targets.end());
    }

    void optimize() {
        // Sort the adj list for better cache locality.
        for (auto &adjList : adj) {
            adjList.shrink_to_fit();
            std::sort(adjList.begin(), adjList.end(), std::less<>());
        }
    }

    [[nodiscard]] bool edgeExists(Vertex source, Vertex target) const {
        auto& adjList = adj[source];
        auto it = std::find(adjList.begin(), adjList.end(), target);
        return (it != adjList.end());
    }

    [[nodiscard]] const std::vector<Vertex> &getConnected(Vertex source) const {
        return adj[source];
    }
};

std::ostream &operator <<(std::ostream &out, const Graph &path);