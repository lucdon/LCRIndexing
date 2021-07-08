#pragma once

#include "graphs/Query.hpp"

class QueryWriter {
public:
    QueryWriter() = default;
    virtual ~QueryWriter() = default;

    QueryWriter(const QueryWriter &) = default;
    QueryWriter(QueryWriter &&) = default;

    QueryWriter &operator =(const QueryWriter &) = default;
    QueryWriter &operator =(QueryWriter &&) = default;

    virtual const std::string &fileName() = 0;

    virtual bool writeQueries(const ReachQuerySet &queries, const std::string &filePath) = 0;
    virtual bool writeLabeledQueries(const LCRQuerySet &queries, const std::string &filePath) = 0;

    static std::unique_ptr<QueryWriter> createQueryWriter();
};