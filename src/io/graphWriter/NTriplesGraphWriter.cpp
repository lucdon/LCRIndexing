#include "NTriplesGraphWriter.hpp"

bool NTriplesGraphWriter::writeGraph(const DiGraph &graph, const std::string &filePath) {
    std::ofstream graphFile { filePath };

    if (graphFile.fail()) {
        std::cerr << "Failed to open file: " << filePath << " for write" << std::fatal;
        return false;
    }

    graphFile << graph.getVertexCount() << "," << graph.getEdgeCount() << ",1\n";

    for (auto source = 0u; source < graph.getVertexCount(); source++) {
        for (auto target : graph.getConnected(source)) {
            graphFile << source << " 0 " << target << " .\n";
        }
    }

    std::flush(graphFile);
    return true;
}

bool NTriplesGraphWriter::writeLabeledGraph(const PerLabelGraph &graph, const std::string &filePath) {
    std::ofstream graphFile { filePath };

    if (graphFile.fail()) {
        std::cerr << "Failed to open file: " << filePath << " for write" << std::fatal;
        return false;
    }

    graphFile << graph.getVertexCount() << "," << graph.getEdgeCount() << "," << graph.getLabelCount() << "\n";

    for (auto label = 0; label < graph.getLabelCount(); label++) {
        for (auto source = 0u; source < graph.getVertexCount(); source++) {
            for (auto target : graph.getConnected(source, label)) {
                graphFile << source << " " << label << " " << target << " .\n";
            }
        }
    }

    std::flush(graphFile);
    return true;
}

bool NTriplesGraphWriter::writeLabeledGraph(const LabeledGraph &graph, const std::string &filePath) {
    std::ofstream graphFile { filePath };

    if (graphFile.fail()) {
        std::cerr << "Failed to open file: " << filePath << " for write" << std::fatal;
        return false;
    }

    graphFile << graph.getVertexCount() << "," << graph.getEdgeCount() << "," << graph.getLabelCount() << "\n";

    for (auto source = 0u; source < graph.getVertexCount(); source++) {
        for (auto &target : graph.getConnected(source)) {
            for (auto i = 0u; i < target.second.size(); i++) {
                if (target.second[i]) {
                    graphFile << source << " " << i << " " << target.first << " .\n";
                }
            }
        }
    }

    std::flush(graphFile);
    return true;
}

bool NTriplesGraphWriter::writeLabeledGraph(const LabeledEdgeGraph &graph, const std::string &filePath) {
    std::ofstream graphFile { filePath };

    if (graphFile.fail()) {
        std::cerr << "Failed to open file: " << filePath << " for write" << std::fatal;
        return false;
    }

    graphFile << graph.getVertexCount() << "," << graph.getEdgeCount() << "," << graph.getLabelCount() << "\n";

    for (auto source = 0u; source < graph.getVertexCount(); source++) {
        auto it = graph.getConnected(source);

        while (it.next()) {
            auto &edge = *it;
            graphFile << source << " " << edge.label << " " << edge.target << " .\n";
        }
    }

    std::flush(graphFile);
    return true;
}
