#pragma once

#include "io/GraphReader.hpp"

class CompositeGraphReader : public GraphReader {
private:
    inline static const std::string fileType = "";
    std::vector<std::unique_ptr<GraphReader>> readers;
public:
    explicit CompositeGraphReader(std::vector<std::unique_ptr<GraphReader>> &readers) {
        this->readers.reserve(readers.size());

        for (auto &reader : readers) {
            this->readers.emplace_back(std::move(reader));
        }
    }

    const std::string &fileName() override {
        return fileType;
    }

    std::unique_ptr<DiGraph> readGraph(const std::string &filePath) override;

    std::unique_ptr<PerLabelGraph> readPerLabelGraph(const std::string &filePath) override;
    std::unique_ptr<LabeledGraph> readLabeledGraph(const std::string &filePath) override;
    std::unique_ptr<LabeledEdgeGraph> readLabeledEdgeGraph(const std::string &filePath) override;
};
