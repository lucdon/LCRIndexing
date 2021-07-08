#include <utility/Format.hpp>
#include "Index.hpp"
#include "BFSIndex.hpp"
#include "P2HIndex.hpp"
#include "LandmarkPlusIndex.hpp"
#include "BloomGraphIndex.hpp"
#include "BloomPathIndex.hpp"
#include "BloomInFrequentIndex.hpp"
#include "BloomGlobalMinLabelIndex.hpp"
#include "BFLPathIndex.hpp"
#include "ALCIndex.hpp"
#include "KLCIndex.hpp"
#include "KLCFreqIndex.hpp"
#include "KLCBFLIndex.hpp"
#include "LWBFIndex.hpp"
#include "ScaleHarness.hpp"

namespace lcr {
    std::unique_ptr<Index> Index::create(const std::string &name, std::vector<std::string> &params) {
        std::string lowerCaseName;
        lowerCaseName.resize(name.size());
        std::transform(name.begin(), name.end(), lowerCaseName.begin(), ::tolower);

        if (lowerCaseName == "bfs") {
            return std::make_unique<BFSIndex>();
        }

        if (lowerCaseName == "landmark-plus" || lowerCaseName == "li+") {
            if (params.size() > 2) {
                std::cerr << "Expected at most 2 argument inputs! Name: " << name << std::fatal;
            }

            if (params.empty()) {
                return std::make_unique<LandmarkPlusIndex>(std::numeric_limits<uint32_t>::max(), 0);
            }

            if (params[0] == "custom") {
                if (params.size() == 2) {
                    if (params[1] == "custom") {
                        return std::make_unique<LandmarkPlusIndex>(std::numeric_limits<uint32_t>::max(),
                                                                   std::numeric_limits<uint32_t>::max());
                    }

                    return std::make_unique<LandmarkPlusIndex>(std::numeric_limits<uint32_t>::max(),
                                                               std::stoul(params[1]));
                } else {
                    return std::make_unique<LandmarkPlusIndex>(std::numeric_limits<uint32_t>::max(), 0);
                }
            } else {
                if (params.size() == 2) {
                    if (params[1] == "custom") {
                        return std::make_unique<LandmarkPlusIndex>(std::stoul(params[0]),
                                                                   std::numeric_limits<uint32_t>::max());
                    }

                    return std::make_unique<LandmarkPlusIndex>(std::stoul(params[0]), std::stoul(params[1]));
                } else {
                    return std::make_unique<LandmarkPlusIndex>(std::stoul(params[0]), 0);
                }
            }
        }

        if (lowerCaseName == "lwbf") {
            uint32_t i = std::numeric_limits<uint32_t>::max();
            uint32_t j = std::numeric_limits<uint32_t>::max();
            uint32_t k = std::numeric_limits<uint32_t>::max();

            if (params.empty()) {
                return std::make_unique<LWBFIndex>(i, j, k);
            }

            if (params.size() != 3) {
                std::cerr << "Expected 0 or 3 arguments as input! Name: " << name << std::fatal;
            }

            if (params[0] != "custom") {
                i = uint32_t(std::stoll(params[0]));
            }

            if (params[1] != "custom") {
                j = uint32_t(std::stoll(params[1]));
            }

            if (params[2] != "custom") {
                k = uint32_t(std::stoll(params[2]));
            }

            return std::make_unique<LWBFIndex>(i, j, k);
        }

        if (lowerCaseName == "bfl-path" || lowerCaseName == "bfflp") {
            if (params.size() != 1) {
                std::cerr << "Expected single argument input! Name: " << name << std::fatal;
            }

            return std::make_unique<BFLPathIndex>(uint32_t(std::stoll(params[0])));
        }

        if (lowerCaseName == "bgi") {
            if (params.size() != 1) {
                std::cerr << "Expected single argument input! Name: " << name << std::fatal;
            }

            return std::make_unique<BloomGraphIndex>(uint32_t(std::stoll(params[0])));
        }

        if (lowerCaseName == "bpi") {
            if (params.size() != 1) {
                std::cerr << "Expected single argument input! Name: " << name << std::fatal;
            }

            return std::make_unique<BloomPathIndex>(uint32_t(std::stoll(params[0])));
        }

        if (lowerCaseName == "bfi") {
            if (params.size() != 1) {
                std::cerr << "Expected single argument input! Name: " << name << std::fatal;
            }

            return std::make_unique<BloomInFrequentIndex>(uint32_t(std::stoll(params[0])));
        }

        if (lowerCaseName == "bgmi") {
            if (params.size() != 1) {
                std::cerr << "Expected single argument input! Name: " << name << std::fatal;
            }

            return std::make_unique<BloomGlobalMinLabelIndex>(uint32_t(std::stoll(params[0])));
        }

        if (lowerCaseName == "alc") {
            if (params.size() > 2 || params.empty()) {
                std::cerr << "Expected ALC reachIndexName [args] inputs! Name: " << name << std::fatal;
            }

            if (params.size() == 1) {
                return std::make_unique<ALCIndex>(params[0], 0);
            } else {
                return std::make_unique<ALCIndex>(params[0], uint32_t(std::stoll(params[1])));
            }
        }

        if (lowerCaseName == "klc") {
            if (params.size() > 3 || params.empty()) {
                std::cerr << "Expected KLC [<k>] <reachIndexName> [<args>]! Name: " << name << std::fatal;
            }

            auto &firstParam = params[0];
            uint32_t k = std::numeric_limits<uint32_t>::max();
            auto nextParam = 0;

            if (!firstParam.empty() &&
                std::find_if(firstParam.begin(), firstParam.end(), [](auto c) { return !std::isdigit(c); }) ==
                firstParam.end()) {
                k = std::stoul(firstParam);
                nextParam = 1;
            }

            if (params.size() <= nextParam) {
                std::cerr << "Expected KLC [<k>] <reachIndexName> [<args>]! Name: " << name << std::fatal;
            }

            auto &secondParam = params[nextParam++];

            if (params.size() <= nextParam) {
                return std::make_unique<KLCIndex>(k, secondParam, 0);
            } else {
                return std::make_unique<KLCIndex>(k, secondParam, std::stoul(params[nextParam]));
            }
        }

        if (lowerCaseName == "klcfreq" || lowerCaseName == "klc-freq") {
            if (params.size() > 3 || params.empty()) {
                std::cerr << "Expected KLCFREQ [<k>] <reachIndexName> [<args>]! Name: " << name << std::fatal;
            }

            auto &firstParam = params[0];
            uint32_t k = std::numeric_limits<uint32_t>::max();
            auto nextParam = 0;

            if (!firstParam.empty() &&
                std::find_if(firstParam.begin(), firstParam.end(), [](auto c) { return !std::isdigit(c); }) ==
                firstParam.end()) {
                k = std::stoul(firstParam);
                nextParam = 1;
            }

            if (params.size() <= nextParam) {
                std::cerr << "Expected KLCFREQ [<k>] <reachIndexName> [<args>]! Name: " << name << std::fatal;
            }

            auto &secondParam = params[nextParam++];

            if (params.size() <= nextParam) {
                return std::make_unique<KLCFreqIndex>(k, secondParam, 0);
            } else {
                return std::make_unique<KLCFreqIndex>(k, secondParam, std::stoul(params[nextParam]));
            }
        }

        if (lowerCaseName == "klcbfl" || lowerCaseName == "klc-bfl") {
            uint32_t k = std::numeric_limits<uint32_t>::max();

            if (!params.empty()) {
                k = std::stoul(params[0]);
            }

            return std::make_unique<KLCBFLIndex>(k);
        }

        if (lowerCaseName == "p2h" || lowerCaseName == "pruned-2-hop" || lowerCaseName == "ph+") {
            if (params.empty()) {
                return std::make_unique<P2HIndex>();
            }

            return std::make_unique<P2HIndex>(std::stoul(params[0]));
        }

        if (lowerCaseName == "scale-harness" || lowerCaseName == "sh") {
            if (params.empty()) {
                std::cerr << "Expected SH [<w>] <indexName> [<args>]! Name: " << name << std::fatal;
            }

            uint32_t w = 12;
            std::string indexName;
            std::vector<std::string> indexParams;

            auto &firstParam = params[0];
            auto nextParam = 0;

            if (!firstParam.empty() &&
                std::find_if(firstParam.begin(), firstParam.end(), [](auto c) { return !std::isdigit(c); }) ==
                firstParam.end()) {
                w = std::stoul(firstParam);
                nextParam = 1;
            }

            if (params.size() <= nextParam) {
                std::cerr << "Expected SH [<w>] <indexName> [<args>]! Name: " << name << std::fatal;
            }

            indexName = params[nextParam];

            for (auto i = nextParam + 1u; i < params.size(); i++) {
                indexParams.emplace_back(params[i]);
            }

            return std::make_unique<ScaleHarness>(indexName, indexParams, w);
        }

        std::cerr << "Unknown reachability index! Name: " << name << std::fatal;
        return nullptr;
    }

    std::ostream &operator <<(std::ostream &out, const Index &index) {
        formatWidth(out, index.getName(), 50);
        out << "size: ";
        formatMemory(out, index.indexSize());
        return out;
    }
}