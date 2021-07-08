#include "DotGraphWriter.hpp"

bool DotGraphWriter::writeGraph(const DiGraph &graph, const std::string &filePath) {
    std::ofstream graphFile { filePath };

    if (graphFile.fail()) {
        std::cerr << "Failed to open file: " << filePath << " for write" << std::fatal;
        return false;
    }

    graphFile << "digraph \n";
    graphFile << "{\n";
    graphFile << "graph [overlap=scale];\n";

    for (auto vertex = 0u; vertex < graph.getVertexCount(); vertex++) {
        graphFile << vertex << " [label=\"" << vertex << "\"];\n";
    }

    for (auto source = 0u; source < graph.getVertexCount(); source++) {
        for (auto target : graph.getConnected(source)) {
            graphFile << source << " -> " << target << ";\n";
        }
    }

    graphFile << "}";

    std::flush(graphFile);
    return true;
}

bool DotGraphWriter::writeLabeledGraph(const PerLabelGraph &graph, const std::string &filePath) {
    std::ofstream graphFile { filePath };

    if (graphFile.fail()) {
        std::cerr << "Failed to open file: " << filePath << " for write" << std::fatal;
        return false;
    }

    graphFile << "digraph \n";
    graphFile << "{\n";
    graphFile << "graph [overlap=scale];\n";

    for (auto vertex = 0u; vertex < graph.getVertexCount(); vertex++) {
        graphFile << vertex << " [label=\"" << vertex << "\"];\n";
    }

    for (auto label = 0; label < graph.getLabelCount(); label++) {
        for (auto source = 0u; source < graph.getVertexCount(); source++) {
            for (auto target : graph.getConnected(source, label)) {
                graphFile << source << " -> " << target << "[label=\"" << label << "\"]" << ";\n";
            }
        }
    }

    graphFile << "}";

    std::flush(graphFile);
    return true;
}

bool DotGraphWriter::writeLabeledGraph(const LabeledGraph &graph, const std::string &filePath) {
    std::ofstream graphFile { filePath };

    if (graphFile.fail()) {
        std::cerr << "Failed to open file: " << filePath << " for write" << std::fatal;
        return false;
    }

    graphFile << "digraph \n";
    graphFile << "{\n";
    graphFile << "graph [overlap=scale];\n";

    for (auto vertex = 0u; vertex < graph.getVertexCount(); vertex++) {
        graphFile << vertex << " [label=\"" << vertex << "\"];\n";
    }

    for (auto source = 0u; source < graph.getVertexCount(); source++) {
        for (auto &target : graph.getConnected(source)) {
            for (auto i = 0u; i < target.second.size(); i++) {
                if (target.second[i]) {
                    graphFile << source << " -> " << target.first << "[label=\"" << i << "\"]" << ";\n";
                }
            }
        }
    }

    graphFile << "}";

    std::flush(graphFile);
    return true;
}

bool DotGraphWriter::writeLabeledGraph(const LabeledEdgeGraph &graph, const std::string &filePath) {
    std::ofstream graphFile { filePath };

    if (graphFile.fail()) {
        std::cerr << "Failed to open file: " << filePath << " for write" << std::fatal;
        return false;
    }

    graphFile << "digraph \n";
    graphFile << "{\n";
    graphFile << "graph [overlap=scale];\n";

    for (auto vertex = 0u; vertex < graph.getVertexCount(); vertex++) {
        graphFile << vertex << " [label=\"" << vertex << "\"];\n";
    }

    for (auto source = 0u; source < graph.getVertexCount(); source++) {
        auto it = graph.getConnected(source);

        while (it.next()) {
            auto &edge = *it;
            graphFile << source << " -> " << edge.target << "[label=\"" << edge.label << "\"]" << ";\n";
        }
    }

    graphFile << "}";

    std::flush(graphFile);
    return true;
}
