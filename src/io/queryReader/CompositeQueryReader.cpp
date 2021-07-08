#include "CompositeQueryReader.hpp"
#include "CSVQueryReader.hpp"

inline static bool endsWith(std::string const &value, std::string const &ending) {
    if (ending.size() > value.size()) {
        return false;
    }

    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::unique_ptr<ReachQuerySet> CompositeQueryReader::readQueries(const std::string &filePath) {
    for (auto &reader : this->readers) {
        if (endsWith(filePath, reader->fileName())) {
            return reader->readQueries(filePath);
        }
    }

    std::cerr << "Failed to read queries! No matching queryReader for file extension! path: " << filePath << std::fatal;
    return nullptr;
}

std::unique_ptr<LCRQuerySet> CompositeQueryReader::readLabeledQueries(const std::string &filePath) {
    for (auto &reader : this->readers) {
        if (endsWith(filePath, reader->fileName())) {
            return reader->readLabeledQueries(filePath);
        }
    }

    std::cerr << "Failed to read queries! No matching queryReader for file extension! path: " << filePath << std::fatal;
    return nullptr;
}

std::unique_ptr<QueryReader> QueryReader::createQueryReader() {
    std::vector<std::unique_ptr<QueryReader>> graphReaders;
    graphReaders.emplace_back(std::make_unique<CSVQueryReader>());

    return std::make_unique<CompositeQueryReader>(graphReaders);
}