#include "Query.hpp"

std::ostream &operator <<(std::ostream &out, const ReachQuery &query) {
    out << "ASK { ?s " << query.source << " ?p ?t " << query.target << " }";
    return out;
}

std::ostream &operator <<(std::ostream &out, const LCRQuery &query) {
    out << "ASK { ?s " << query.source << " ?p {";

    bool first = true;

    for (uint32_t i = 0; i < query.labelSet.size(); i++) {
        if (query.labelSet[i]) {
            if (first) {
                first = false;
            } else {
                out << ", ";
            }

            out << i;
        }
    }

    out << "} ?t " << query.target << " }";
    return out;
}

std::ostream &printLabels(std::ostream &out, const LabelSet &labelSet) {
    out << "{";

    bool first = true;

    for (uint32_t i = 0; i < labelSet.size(); i++) {
        if (labelSet[i]) {
            if (first) {
                first = false;
            } else {
                out << ", ";
            }

            out << i;
        }
    }

    out << "}";
    return out;
}

std::ostream &operator <<(std::ostream &out, const ReachQuerySet &queries) {
    if (queries.empty()) {
        return out;
    }

    out << "querySet: [\n    ";
    out << queries[0];

    for (auto i = 1u; i < queries.size(); i++) {
        out << "\n    " << queries[i];
    }
    out << "\n]";

    return out;
}

std::ostream &operator <<(std::ostream &out, const LCRQuerySet &queries) {
    if (queries.empty()) {
        return out;
    }

    out << "querySet: [\n    ";
    out << queries[0];

    for (auto i = 1u; i < queries.size(); i++) {
        out << "\n    " << queries[i];
    }
    out << "\n]";

    return out;
}