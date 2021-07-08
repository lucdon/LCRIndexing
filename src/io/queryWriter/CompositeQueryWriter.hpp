#pragma once

#include "io/QueryWriter.hpp"

class CompositeQueryWriter : public QueryWriter {
private:
    inline static const std::string fileType = "";
    std::vector<std::unique_ptr<QueryWriter>> writers;
public:
    explicit CompositeQueryWriter(std::vector<std::unique_ptr<QueryWriter>> &writers) {
        this->writers.reserve(writers.size());

        for (auto &writer : writers) {
            this->writers.emplace_back(std::move(writer));
        }
    }

    const std::string &fileName() override {
        return fileType;
    }

    bool writeQueries(const ReachQuerySet &queries, const std::string &filePath) override;
    bool writeLabeledQueries(const LCRQuerySet &queries, const std::string &filePath) override;
};
