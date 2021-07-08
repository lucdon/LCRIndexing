#pragma once

#include "io/GraphWriter.hpp"

class CompositeGraphWriter : public GraphWriter {
private:
    inline static const std::string fileType = "";
    std::vector<std::unique_ptr<GraphWriter>> writers;
public:
    explicit CompositeGraphWriter(std::vector<std::unique_ptr<GraphWriter>> &writers) {
        this->writers.reserve(writers.size());

        for (auto &writer : writers) {
            this->writers.emplace_back(std::move(writer));
        }
    }

    const std::string &fileName() override {
        return fileType;
    }

    bool writeGraph(const DiGraph& graph, const std::string& filePath) override;

    bool writeLabeledGraph(const PerLabelGraph& graph, const std::string& filePath) override;
    bool writeLabeledGraph(const LabeledGraph& graph, const std::string& filePath) override;
    bool writeLabeledGraph(const LabeledEdgeGraph& graph, const std::string& filePath) override;
};
