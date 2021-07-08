#include "CompositeGraphReader.hpp"
#include "NTriplesGraphReader.hpp"
#include "RdfGraphReader.hpp"
#include "EdgeGraphReader.hpp"

inline static bool endsWith(std::string const &value, std::string const &ending) {
    if (ending.size() > value.size()) {
        return false;
    }

    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::unique_ptr<DiGraph> CompositeGraphReader::readGraph(const std::string &filePath) {
    for (auto &reader : this->readers) {
        if (endsWith(filePath, reader->fileName())) {
            auto graph = reader->readGraph(filePath);
            graph->optimize();
            return graph;
        }
    }

    std::cerr << "Failed to read graph! No matching graphReader for file extension! File: " << filePath << std::fatal;
    return nullptr;
}

std::unique_ptr<LabeledGraph> CompositeGraphReader::readLabeledGraph(const std::string &filePath) {
    for (auto &reader : this->readers) {
        if (endsWith(filePath, reader->fileName())) {
            auto graph = reader->readLabeledGraph(filePath);
            graph->optimize();
            return graph;
        }
    }

    std::cerr << "Failed to read graph! No matching graphReader for file extension! File: " << filePath << std::fatal;
    return nullptr;
}

std::unique_ptr<LabeledEdgeGraph> CompositeGraphReader::readLabeledEdgeGraph(const std::string &filePath) {
    for (auto &reader : this->readers) {
        if (endsWith(filePath, reader->fileName())) {
            auto graph = reader->readLabeledEdgeGraph(filePath);
            graph->optimize();
            return graph;
        }
    }

    std::cerr << "Failed to read graph! No matching graphReader for file extension! File: " << filePath << std::fatal;
    return nullptr;
}

std::unique_ptr<PerLabelGraph> CompositeGraphReader::readPerLabelGraph(const std::string &filePath) {
    for (auto &reader : this->readers) {
        if (endsWith(filePath, reader->fileName())) {
            auto graph = reader->readPerLabelGraph(filePath);
            graph->optimize();
            return graph;
        }
    }

    std::cerr << "Failed to read graph! No matching graphReader for file extension! File: " << filePath << std::fatal;
    return nullptr;
}

std::unique_ptr<GraphReader> GraphReader::createGraphReader() {
    std::vector<std::unique_ptr<GraphReader>> graphReaders;
    graphReaders.emplace_back(std::make_unique<NTriplesGraphReader>());
    graphReaders.emplace_back(std::make_unique<RdfGraphReader>());
    graphReaders.emplace_back(std::make_unique<EdgeGraphReader>());

    return std::make_unique<CompositeGraphReader>(graphReaders);
}