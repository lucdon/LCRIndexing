#pragma once

#include <graphs/LabeledGraph.hpp>
#include "graphs/DiGraph.hpp"

class GraphWriter {
public:
    GraphWriter() = default;
    virtual ~GraphWriter() = default;

    GraphWriter(const GraphWriter &) = default;
    GraphWriter(GraphWriter &&) = default;

    GraphWriter &operator =(const GraphWriter &) = default;
    GraphWriter &operator =(GraphWriter &&) = default;

    virtual const std::string& fileName() = 0;

    virtual bool writeGraph(const DiGraph& graph, const std::string& filePath) = 0;

    virtual bool writeLabeledGraph(const PerLabelGraph& graph, const std::string& filePath) = 0;
    virtual bool writeLabeledGraph(const LabeledGraph& graph, const std::string& filePath) = 0;
    virtual bool writeLabeledGraph(const LabeledEdgeGraph& graph, const std::string& filePath) = 0;

    static std::unique_ptr<GraphWriter> createGraphWriter();
};