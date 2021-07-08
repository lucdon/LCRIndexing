#pragma once

#include "io/QueryReader.hpp"

class CSVQueryReader : public QueryReader {
private:
    inline static const std::string fileType = ".csv";
public:
    const std::string &fileName() override {
        return fileType;
    }

    std::unique_ptr<ReachQuerySet> readQueries(const std::string &filePath) override;
    std::unique_ptr<LCRQuerySet> readLabeledQueries(const std::string &filePath) override;
};
