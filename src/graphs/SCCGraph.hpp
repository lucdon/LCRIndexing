#pragma once

#include "DiGraph.hpp"

class SCCGraph {
private:
    std::unique_ptr<DiGraph> componentGraph;

    std::vector<Vertex> vertexMapping;
    std::vector<std::vector<Vertex>> componentMapping;

    size_t componentCount = 0;

public:
    SCCGraph(const SCCGraph &) = delete;
    SCCGraph(SCCGraph &&) = default;

    SCCGraph &operator =(const SCCGraph &) = delete;
    SCCGraph &operator =(SCCGraph &&) = default;

    explicit SCCGraph(std::unique_ptr<DiGraph> componentGraph, std::vector<uint32_t> &&vertexMapping) : vertexMapping(
            std::move(vertexMapping)), componentGraph(std::move(componentGraph)), componentMapping(0) {
        componentCount = this->componentGraph->getVertexCount();
    }

    explicit SCCGraph(std::unique_ptr<DiGraph> componentGraph, std::vector<uint32_t> &&vertexMapping,
                      std::vector<std::vector<Vertex>> &&componentMapping) : vertexMapping(std::move(vertexMapping)),
                                                                             componentGraph(std::move(componentGraph)),
                                                                             componentMapping(
                                                                                     std::move(componentMapping)) {
        componentCount = this->componentGraph->getVertexCount();
    }

    void clearComponentMapping() {
        componentMapping.clear();
        componentMapping.shrink_to_fit();
    }

    void clearComponentGraph() {
        componentGraph = nullptr;
    }

    [[nodiscard]] size_t getComponentCount() const {
        return componentCount;
    }

    [[nodiscard]] size_t getOriginalVertexCount() const {
        return vertexMapping.size();
    }

    [[nodiscard]] size_t getSizeInBytes() const {
        if (componentGraph == nullptr) {
            return vertexMapping.size() * sizeof(Vertex);
        }

        return componentGraph->getSizeInBytes() + vertexMapping.size() * sizeof(Vertex);
    }

    [[nodiscard]] bool hasComponentGraph() const { return componentGraph != nullptr; }
    [[nodiscard]] bool hasComponentMapping() const { return !componentMapping.empty(); }

    [[nodiscard]] const DiGraph &getComponentGraph() const {
        return *componentGraph;
    }

    // Note only present if requested during tarjanSCC
    [[nodiscard]] const std::vector<Vertex> &getVerticesForComponent(uint32_t componentIndex) const {
        return componentMapping[componentIndex];
    }

    [[nodiscard]] uint32_t getComponentIndex(Vertex vertex) const {
        return vertexMapping[vertex];
    }

    [[nodiscard]] bool isSingleComponent(Vertex first) const {
        uint32_t firstIndex = vertexMapping[first];

        if (componentMapping[firstIndex].size() != 1) {
            return false;
        }

        return true;
    }

    template<class ... Args>
    [[nodiscard]] bool isSingleComponent(Vertex first, Args ... args) const {
        const auto list = { args... };
        uint32_t firstIndex = vertexMapping[first];

        if (componentMapping[firstIndex].size() != list.size() + 1) {
            return false;
        }

        return std::all_of(list.begin(), list.end(), [firstIndex, this](Vertex vertex) {
            return firstIndex == this->vertexMapping[vertex];
        });
    }

    template<class ... Args>
    [[nodiscard]] bool areInSameComponent(Vertex first, Args ... args) const {
        const auto list = { args... };
        uint32_t firstIndex = vertexMapping[first];

        return std::all_of(list.begin(), list.end(), [firstIndex, this](Vertex vertex) {
            return firstIndex == this->vertexMapping[vertex];
        });
    }
};

void componentDistribution(const SCCGraph &graph, std::vector<std::pair<uint32_t, Vertex>> &distribution);
std::ostream &printComponentDistribution(std::ostream &out, const SCCGraph &graph);
std::ostream &operator <<(std::ostream &out, const SCCGraph &graph);