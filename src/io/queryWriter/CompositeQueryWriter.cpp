#include "CompositeQueryWriter.hpp"
#include "CSVQueryWriter.hpp"

inline static bool endsWith(std::string const &value, std::string const &ending) {
    if (ending.size() > value.size()) {
        return false;
    }

    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

bool CompositeQueryWriter::writeQueries(const ReachQuerySet &queries, const std::string &filePath) {
    for (auto &writer : this->writers) {
        if (endsWith(filePath, writer->fileName())) {
#if FILESYSTEM_ENABLED == 1
            std::filesystem::path path(filePath);
            std::filesystem::create_directories(path.parent_path());
#endif
            return writer->writeQueries(queries, filePath);
        }
    }

    std::cerr << "Failed to write queries! No matching queryWriter for file extension! file: " << filePath
              << std::fatal;
    return false;
}

bool CompositeQueryWriter::writeLabeledQueries(const LCRQuerySet &queries, const std::string &filePath) {
    for (auto &writer : this->writers) {
        if (endsWith(filePath, writer->fileName())) {
#if FILESYSTEM_ENABLED == 1
            std::filesystem::path path(filePath);
            std::filesystem::create_directories(path.parent_path());
#endif
            return writer->writeLabeledQueries(queries, filePath);
        }
    }

    std::cerr << "Failed to write queries! No matching queryWriter for file extension! file: " << filePath
              << std::fatal;
    return false;
}

std::unique_ptr<QueryWriter> QueryWriter::createQueryWriter() {
    std::vector<std::unique_ptr<QueryWriter>> graphWriters;
    graphWriters.emplace_back(std::make_unique<CSVQueryWriter>());

    return std::make_unique<CompositeQueryWriter>(graphWriters);
}