#pragma once

#include "graphs/Query.hpp"

class QueryReader {
public:
    QueryReader() = default;
    virtual ~QueryReader() = default;

    QueryReader(const QueryReader &) = default;
    QueryReader(QueryReader &&) = default;

    QueryReader &operator =(const QueryReader &) = default;
    QueryReader &operator =(QueryReader &&) = default;

    virtual const std::string &fileName() = 0;

    virtual std::unique_ptr<ReachQuerySet> readQueries(const std::string &filePath) = 0;
    virtual std::unique_ptr<LCRQuerySet> readLabeledQueries(const std::string &filePath) = 0;

    static std::unique_ptr<QueryReader> createQueryReader();
};
