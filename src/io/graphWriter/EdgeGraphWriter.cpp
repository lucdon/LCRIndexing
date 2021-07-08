#include "EdgeGraphWriter.hpp"

bool EdgeGraphWriter::writeGraph(const DiGraph &graph, const std::string &filePath) {
    std::ofstream graphFile { filePath };

    if (graphFile.fail()) {
        std::cerr << "Failed to open file: " << filePath << " for write" << std::fatal;
        return false;
    }

    for (auto source = 0u; source < graph.getVertexCount(); source++) {
        for (auto target : graph.getConnected(source)) {
            graphFile << source << " " << target << " 0\n";
        }
    }

    std::flush(graphFile);
    return true;
}

bool EdgeGraphWriter::writeLabeledGraph(const PerLabelGraph &graph, const std::string &filePath) {
    std::ofstream graphFile { filePath };

    if (graphFile.fail()) {
        std::cerr << "Failed to open file: " << filePath << " for write" << std::fatal;
        return false;
    }

    for (auto label = 0; label < graph.getLabelCount(); label++) {
        for (auto source = 0u; source < graph.getVertexCount(); source++) {
            for (auto target : graph.getConnected(source, label)) {
                graphFile << source << " " << target << " " << label << "\n";
            }
        }
    }

    std::flush(graphFile);
    return true;
}

bool EdgeGraphWriter::writeLabeledGraph(const LabeledGraph &graph, const std::string &filePath) {
    std::ofstream graphFile { filePath };

    if (graphFile.fail()) {
        std::cerr << "Failed to open file: " << filePath << " for write" << std::fatal;
        return false;
    }

    for (auto source = 0u; source < graph.getVertexCount(); source++) {
        for (auto &target : graph.getConnected(source)) {
            for (auto i = 0u; i < target.second.size(); i++) {
                if (target.second[i]) {
                    graphFile << source << " " << target.first << " " << i << "\n";
                }
            }
        }
    }

    std::flush(graphFile);
    return true;
}

bool EdgeGraphWriter::writeLabeledGraph(const LabeledEdgeGraph &graph, const std::string &filePath) {
    std::ofstream graphFile { filePath };

    if (graphFile.fail()) {
        std::cerr << "Failed to open file: " << filePath << " for write" << std::fatal;
        return false;
    }

    for (auto source = 0u; source < graph.getVertexCount(); source++) {
        auto it = graph.getConnected(source);

        while (it.next()) {
            auto &edge = *it;
            graphFile << source << " " << edge.target << " " << edge.label << "\n";
        }
    }

    std::flush(graphFile);
    return true;
}
