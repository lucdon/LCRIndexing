#include "CSVQueryWriter.hpp"

bool CSVQueryWriter::writeQueries(const ReachQuerySet &queries, const std::string &filePath) {
    std::ofstream queriesFile { filePath };

    if (queriesFile.fail()) {
        std::cerr << "Failed to write queries! Could not open file!" << std::fatal;
    }

    for (auto &query : queries) {
        queriesFile << query.source << ",0+," << query.target << "\n";
    }

    std::flush(queriesFile);
    return true;
}

bool CSVQueryWriter::writeLabeledQueries(const LCRQuerySet &queries, const std::string &filePath) {
    std::ofstream queriesFile { filePath };

    if (queriesFile.fail()) {
        std::cerr << "Failed to write queries! Could not open file!" << std::fatal;
    }

    for (auto &query : queries) {
        queriesFile << query.source << ",";

        bool first = true;

        for (auto i = 0u; i < query.labelSet.size(); i++) {
            if (!query.labelSet[i]) {
                continue;
            }

            if (first) {
                first = false;
            } else {
                queriesFile << "+";
            }

            queriesFile << i;
        }

        queriesFile << "," << query.target << "\n";
    }

    std::flush(queriesFile);
    return true;
}
