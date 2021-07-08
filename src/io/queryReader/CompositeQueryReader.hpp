#pragma once

#include "io/QueryReader.hpp"

class CompositeQueryReader : public QueryReader {
private:
    inline static const std::string fileType = "";
    std::vector<std::unique_ptr<QueryReader>> readers;
public:
    explicit CompositeQueryReader(std::vector<std::unique_ptr<QueryReader>> &readers) {
        this->readers.reserve(readers.size());

        for (auto &reader : readers) {
            this->readers.emplace_back(std::move(reader));
        }
    }

    const std::string &fileName() override {
        return fileType;
    }

    std::unique_ptr<ReachQuerySet> readQueries(const std::string &filePath) override;
    std::unique_ptr<LCRQuerySet> readLabeledQueries(const std::string &filePath) override;
};
