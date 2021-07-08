#pragma once

#include "io/GraphWriter.hpp"

class NTriplesGraphWriter : public GraphWriter {
private:
    inline static const std::string fileType = ".nt";
public:
    const std::string &fileName() override {
        return fileType;
    }

    bool writeGraph(const DiGraph& graph, const std::string& filePath) override;

    bool writeLabeledGraph(const PerLabelGraph& graph, const std::string& filePath) override;
    bool writeLabeledGraph(const LabeledGraph& graph, const std::string& filePath) override;
    bool writeLabeledGraph(const LabeledEdgeGraph& graph, const std::string& filePath) override;
};
