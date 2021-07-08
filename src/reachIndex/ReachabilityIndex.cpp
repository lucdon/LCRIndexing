#include <utility/Format.hpp>
#include "BFSIndex.hpp"
#include "DFSIndex.hpp"
#include "BiBFSIndex.hpp"
#include "PLLIndex.hpp"
#include "PPLIndex.hpp"
#include "TCIndex.hpp"
#include "BFLIndex.hpp"
#include "HybridBFSIndex.hpp"
#include "BFLOnceIndex.hpp"

std::unique_ptr<ReachabilityIndex>
ReachabilityIndex::create(const std::string &name, std::vector<std::string>& params) {
    std::string lowerCaseName;
    lowerCaseName.resize(name.size());
    std::transform(name.begin(), name.end(), lowerCaseName.begin(), ::tolower);

    if (lowerCaseName == "control") {
        return std::make_unique<BFSIndex>();
    }

    if (lowerCaseName == "bfs") {
        return std::make_unique<BFSIndex>();
    }

    if (lowerCaseName == "hybrid-bfs") {
        return std::make_unique<HybridBFSIndex>();
    }

    if (lowerCaseName == "dfs") {
        return std::make_unique<DFSIndex>();
    }

    if (lowerCaseName == "bibfs") {
        return std::make_unique<BiBFSIndex>();
    }

    if (lowerCaseName == "tc") {
        return std::make_unique<TCIndex>();
    }

    if (lowerCaseName == "pll") {
        return std::make_unique<PLLIndex>();
    }

    if (lowerCaseName == "ppl") {
        return std::make_unique<PPLIndex>();
    }

    if (lowerCaseName == "bfl") {
        return std::make_unique<BFLIndex>(uint32_t(std::stoll(params[0])));
    }

    if (lowerCaseName == "bfl-once") {
        return std::make_unique<BFLOnceIndex>(uint32_t(std::stoll(params[0])));
    }

    std::cerr << "Unknown reachability index! Name: " << name << std::fatal;
    return nullptr;
}

std::ostream &operator <<(std::ostream &out, const ReachabilityIndex &index) {
    formatWidth(out, index.getName(), 50);
    out << "size: ";
    formatMemory(out, index.indexSize());
    return out;
}