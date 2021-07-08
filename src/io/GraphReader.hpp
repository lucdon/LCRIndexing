#pragma once

#include <graphs/LabeledGraph.hpp>
#include "graphs/DiGraph.hpp"

class GraphReader {
public:
    GraphReader() = default;
    virtual ~GraphReader() = default;

    GraphReader(const GraphReader &) = default;
    GraphReader(GraphReader &&) = default;

    GraphReader &operator =(const GraphReader &) = default;
    GraphReader &operator =(GraphReader &&) = default;

    virtual const std::string& fileName() = 0;

    virtual std::unique_ptr<DiGraph> readGraph(const std::string& filePath) = 0;

    virtual std::unique_ptr<PerLabelGraph> readPerLabelGraph(const std::string& filePath) = 0;
    virtual std::unique_ptr<LabeledGraph> readLabeledGraph(const std::string& filePath) = 0;
    virtual std::unique_ptr<LabeledEdgeGraph> readLabeledEdgeGraph(const std::string &filePath) = 0;

    static std::unique_ptr<GraphReader> createGraphReader();
};
