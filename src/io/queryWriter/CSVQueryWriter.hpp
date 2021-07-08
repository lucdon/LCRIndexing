#pragma once

#include "io/QueryWriter.hpp"

class CSVQueryWriter : public QueryWriter {
private:
    inline static const std::string fileType = ".csv";
public:
    const std::string &fileName() override {
        return fileType;
    }

    bool writeQueries(const ReachQuerySet &queries, const std::string &filePath) override;
    bool writeLabeledQueries(const LCRQuerySet &queries, const std::string &filePath) override;
};
