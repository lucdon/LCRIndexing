#include "CompositeGraphWriter.hpp"
#include "NTriplesGraphWriter.hpp"
#include "DotGraphWriter.hpp"
#include "EdgeGraphWriter.hpp"

inline static bool endsWith(std::string const &value, std::string const &ending) {
    if (ending.size() > value.size()) {
        return false;
    }

    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

bool CompositeGraphWriter::writeGraph(const DiGraph &graph, const std::string &filePath) {
    for (auto &writer : this->writers) {
        if (endsWith(filePath, writer->fileName())) {
#if FILESYSTEM_ENABLED == 1
            std::filesystem::path path(filePath);
            std::filesystem::create_directories(path.parent_path());
#endif
            return writer->writeGraph(graph, filePath);
        }
    }

    std::cerr << "Failed to write graph! No matching graphWriter for file extension! file: " << filePath << std::fatal;
    return false;
}

bool CompositeGraphWriter::writeLabeledGraph(const PerLabelGraph &graph, const std::string &filePath) {
    for (auto &writer : this->writers) {
        if (endsWith(filePath, writer->fileName())) {
#if FILESYSTEM_ENABLED == 1
            std::filesystem::path path(filePath);
            std::filesystem::create_directories(path.parent_path());
#endif
            return writer->writeLabeledGraph(graph, filePath);
        }
    }

    std::cerr << "Failed to write graph! No matching graphWriter for file extension! file: " << filePath << std::fatal;
    return false;
}

bool CompositeGraphWriter::writeLabeledGraph(const LabeledGraph &graph, const std::string &filePath) {
    for (auto &writer : this->writers) {
        if (endsWith(filePath, writer->fileName())) {
#if FILESYSTEM_ENABLED == 1
            std::filesystem::path path(filePath);
            std::filesystem::create_directories(path.parent_path());
#endif
            return writer->writeLabeledGraph(graph, filePath);
        }
    }

    std::cerr << "Failed to write graph! No matching graphWriter for file extension! file: " << filePath << std::fatal;
    return false;
}

bool CompositeGraphWriter::writeLabeledGraph(const LabeledEdgeGraph &graph, const std::string &filePath) {
    for (auto &writer : this->writers) {
        if (endsWith(filePath, writer->fileName())) {
#if FILESYSTEM_ENABLED == 1
            std::filesystem::path path(filePath);
            std::filesystem::create_directories(path.parent_path());
#endif
            return writer->writeLabeledGraph(graph, filePath);
        }
    }

    std::cerr << "Failed to write graph! No matching graphWriter for file extension! file: " << filePath << std::fatal;
    return false;
}

std::unique_ptr<GraphWriter> GraphWriter::createGraphWriter() {
    std::vector<std::unique_ptr<GraphWriter>> graphWriters;
    graphWriters.emplace_back(std::make_unique<NTriplesGraphWriter>());
    graphWriters.emplace_back(std::make_unique<DotGraphWriter>());
    graphWriters.emplace_back(std::make_unique<EdgeGraphWriter>());

    return std::make_unique<CompositeGraphWriter>(graphWriters);
}