#include "SCCGraph.hpp"
#include <utility/Format.hpp>

std::ostream &operator <<(std::ostream &out, const SCCGraph &graph) {
    out << "G_c     = (" << graph.getComponentGraph().getVertexCount() << ", "
        << graph.getComponentGraph().getEdgeCount() << ")\n";

    out << "TC size = ";
    uint64_t size = graph.getComponentGraph().getVertexCount();
    formatMemory(out, size * (size / 8ull));

    return out;
}

void componentDistribution(const SCCGraph &graph, std::vector<std::pair<uint32_t, Vertex>> &distribution) {
    distribution.resize(graph.getComponentCount());

    if (graph.hasComponentMapping()) {
        for (auto component = 0u; component < distribution.size(); component++) {
            distribution[component].first = graph.getVerticesForComponent(component).size();
            distribution[component].second = component;
        }
    } else {
        for (auto component = 0u; component < distribution.size(); component++) {
            distribution[component].first = 0;
            distribution[component].second = component;
        }

        for (auto vertex = 0u; vertex < graph.getOriginalVertexCount(); vertex++) {
            distribution[graph.getComponentIndex(vertex)].first++;
        }
    }

    std::sort(distribution.begin(), distribution.end(), std::greater<>());
}

std::ostream &printComponentDistribution(std::ostream &out, const SCCGraph &graph) {
    std::vector<std::pair<uint32_t, Vertex>> numVerticesByComponent;
    componentDistribution(graph, numVerticesByComponent);

    out << "Component Distribution over, " << graph.getComponentCount() << " components and "
        << graph.getOriginalVertexCount() << " vertices" << std::endl;

    size_t cumSum = 0;
    auto totalVertices = graph.getOriginalVertexCount();

    for (auto i = 0u; i < 25 && i < graph.getComponentCount(); i++) {
        auto &componentPair = numVerticesByComponent[i];

        out << "Index: ";
        formatWidth(out, i, 6);

        out << "Component: ";
        formatWidth(out, componentPair.second, 6);

        out << "Percentage: " << (double(componentPair.first) / double(totalVertices)) * 100 << "%" << std::endl;

        cumSum += componentPair.first;
    }

    out << "Rest:                     Percentage: " << (double(totalVertices - cumSum) / double(totalVertices)) * 100
        << "%" << std::endl;
    out << std::endl;
    return out;
}
