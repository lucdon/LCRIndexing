#pragma once

#include "io/GraphReader.hpp"

// RDF files are the same as .nt files.
// However RDF files are writen with tags instead of ids.
class RdfGraphReader : public GraphReader {
private:
    inline static const std::string fileType = ".rdf";
public:
    const std::string &fileName() override {
        return fileType;
    }

    std::unique_ptr<DiGraph> readGraph(const std::string &filePath) override;

    std::unique_ptr<PerLabelGraph> readPerLabelGraph(const std::string &filePath) override;
    std::unique_ptr<LabeledGraph> readLabeledGraph(const std::string& filePath) override;
    std::unique_ptr<LabeledEdgeGraph> readLabeledEdgeGraph(const std::string &filePath) override;
};
