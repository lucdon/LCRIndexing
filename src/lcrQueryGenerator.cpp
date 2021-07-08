#include <io/GraphReader.hpp>
#include <generator/LCRQuery/WalkerLCRQueryGenerator.hpp>
#include <lcrIndex/Index.hpp>
#include <threading/ThreadPool.hpp>
#include "io/QueryWriter.hpp"
#include "utility/Timer.hpp"
#include "Selector.hpp"
#include "generator/LCRQuery/WalkerStrategies.hpp"

static Timer timer;

std::unique_ptr<LabeledEdgeGraph> readGraph(const std::string &graphFileIn) {
    auto graphReader = GraphReader::createGraphReader();
    timer.begin("read graph");
    auto labeledGraph = graphReader->readLabeledEdgeGraph(graphFileIn);
    timer.end();
    return labeledGraph;
}

void
printQueryStats(const std::string &indexName, std::unordered_map<std::string, std::vector<uint32_t>> &queryResults) {
    auto totalTrue = 0u;
    auto totalFalse = 0u;

    for (auto &pair : queryResults) {
        for (auto value : pair.second) {
            if (value == 1) {
                totalTrue++;
            } else {
                totalFalse++;
            }
        }
    }

    std::cout << "\nQuery stats" << "\n";
    std::cout << "total          = " << (totalTrue + totalFalse) << "\n";
    std::cout << "reachable      = " << totalTrue << "\n";
    std::cout << "not reachable  = " << totalFalse << "\n";
    std::cout << "used evaluator = " << indexName << "\n";
}

void writeQueries(const std::string &graphFileName, const std::vector<LCRQuery> &queries) {
    auto queryWriter = QueryWriter::createQueryWriter();

    auto lastSlash = graphFileName.find_last_of('.');
    auto pathAndNameWithoutExtension = graphFileName.substr(0, lastSlash);

    timer.begin("writing queries");
    queryWriter->writeLabeledQueries(queries, pathAndNameWithoutExtension + ".queries-lcr.csv");
    timer.end();
}

void writeQueries(const std::string &graphFileName,
                  const std::unordered_map<std::string, std::array<std::vector<LCRQuery>, 2>> &queriesByLabel) {
    auto queryWriter = QueryWriter::createQueryWriter();

    timer.begin("writing queries");

    auto lastSlash = graphFileName.find_last_of('.');
    auto pathAndNameWithoutExtension = graphFileName.substr(0, lastSlash);

    for (auto &pair : queriesByLabel) {
        auto &falseQueries = pair.second[0];
        auto &trueQueries = pair.second[1];

        if (!falseQueries.empty()) {
            queryWriter->writeLabeledQueries(falseQueries,
                                             pathAndNameWithoutExtension + ".queries-lcr." + pair.first + ".false.csv");
        }

        if (!trueQueries.empty()) {
            queryWriter->writeLabeledQueries(trueQueries,
                                             pathAndNameWithoutExtension + ".queries-lcr." + pair.first + ".true.csv");
        }
    }

    timer.end();
}

std::vector<LCRQuery>
mergeQueries(LabeledEdgeGraph &labeledEdgeGraph, std::unordered_map<std::string, std::vector<LCRQuery>> &queries,
             bool printStats) {

    if (printStats) {
        auto &threadPool = getThreadPool();

        std::vector<std::function<void(uint32_t id)>> workGroup = { };

        // Do not use bool's here. To prevent race conditions.
        std::unordered_map<std::string, std::vector<uint32_t>> queryResults;

        for (auto &pair : queries) {
            queryResults[pair.first].resize(pair.second.size());
        }

        auto index = lcr::Index::create("BFS");
        index->setGraph(&labeledEdgeGraph);
        index->train();

        for (auto &pair : queries) {
            for (auto i = 0u; i < pair.second.size(); i++) {
                auto &query = pair.second[i];

                workGroup.emplace_back([&queryResults, &index, &pair, i, &query](uint32_t id) {
                    queryResults.at(pair.first)[i] = index->query(query) ? 1u : 0u;
                });
            }
        }

        threadPool.queueWorkGroup(workGroup)->waitTillCompleted();

        printQueryStats(index->getName(), queryResults);
    }

    std::vector<LCRQuery> results;

    for (auto &pair : queries) {
        results.reserve(results.size() + pair.second.size());

        for (auto &query : pair.second) {
            results.emplace_back(query);
        }
    }

    return results;
}

std::unordered_map<std::string, std::array<std::vector<LCRQuery>, 2>>
evaluateQueries(LabeledEdgeGraph &labeledEdgeGraph, std::unordered_map<std::string, std::vector<LCRQuery>> &queries,
                bool printStats) {
    auto &threadPool = getThreadPool();

    std::vector<std::function<void(uint32_t id)>> workGroup = { };

    // Do not use bool's here. To prevent race conditions.
    std::unordered_map<std::string, std::vector<uint32_t>> queryResults;

    for (auto &pair : queries) {
        queryResults[pair.first].resize(pair.second.size());
    }

    auto index = lcr::Index::create("BFS");
    index->setGraph(&labeledEdgeGraph);
    index->train();

    for (auto &pair : queries) {
        for (auto i = 0u; i < pair.second.size(); i++) {
            auto &query = pair.second[i];

            workGroup.emplace_back([&queryResults, &index, &pair, i, &query](uint32_t id) {
                queryResults.at(pair.first)[i] = index->query(query) ? 1u : 0u;
            });
        }
    }

    threadPool.queueWorkGroup(workGroup)->waitTillCompleted();

    if (printStats) {
        printQueryStats(index->getName(), queryResults);
    }

    std::unordered_map<std::string, std::array<std::vector<LCRQuery>, 2>> results;

    for (auto &pair : queryResults) {
        auto &queriesForStr = queries[pair.first];
        auto &resultsForStr = results[pair.first];

        for (auto i = 0u; i < pair.second.size(); i++) {
            resultsForStr[pair.second[i]].emplace_back(queriesForStr[i]);
        }
    }

    return results;
}

struct QueryGenData {
    std::string name;
    uint32_t randomQueriesCount = 0;
    uint32_t connectedQueriesCount = 0;
    uint32_t labelCount = 0;
};

void generateQueries(const LabeledEdgeGraph &labeledGraph,
                     std::unordered_map<std::string, std::vector<LCRQuery>> &queriesOut,
                     std::vector<QueryGenData> &queryGenData) {

    auto &threadPool = getThreadPool();

    std::vector<std::function<void(uint32_t id)>> workGroup = { };

    queriesOut.reserve(queryGenData.size());

    for (auto &genData : queryGenData) {
        if (queriesOut.count(genData.name) != 0) {
            std::cerr << "found duplicate genData name" << std::fatal;
        }

        queriesOut[genData.name].reserve(genData.randomQueriesCount + genData.connectedQueriesCount);
    }

    for (auto &genData : queryGenData) {
        auto &queries = queriesOut.at(genData.name);

        workGroup.emplace_back([&labeledGraph, &queries, &genData](uint32_t id) {
            // Connected generator
            {
                WalkerLCRQueryGenerator queryGenerator(labeledGraph);
                queryGenerator.generate(genData.connectedQueriesCount, genData.labelCount, queries);
            }

            // random generator
            {
                WalkerLCRQueryGenerator queryGenerator(labeledGraph);
                WalkerEmitRandomQueries emitRandomQueries;
                emitRandomQueries.setup(queryGenerator);
                queryGenerator.generate(genData.randomQueriesCount, genData.labelCount, queries);
            }
        });
    }

    threadPool.queueWorkGroup(workGroup)->waitTillCompleted();
}

std::vector<uint32_t> createLabelGenModes(const LabeledEdgeGraph &graph) {
    if (graph.getLabelCount() <= 2) {
        return { 1 };
    }

    if (graph.getLabelCount() <= 4) {
        return { 1, 2 };
    }

    return { std::min(uint32_t(std::floor(double(graph.getLabelCount()) / 4.0)), 2u),
             std::min(uint32_t(std::floor(double(graph.getLabelCount()) / 2.0)), 4u),
             std::min(uint32_t(graph.getLabelCount()) - 2u, 6u) };
}

int main(int argc, char **argv) {
    if (argc < 1) {
        std::cerr << "Usage: --graphFile [graphFileIn] [--printStats] [--allInOne] "
                     "[--splitRandomFromConnected] --randomQueries [count] --connectedQueries [count]" << std::endl;
        return 1;
    }

    bool allInOne = false;
    bool printStats = false;
    bool splitRandomFromConnected = false;

    uint32_t randomQueriesCount = 50000;
    uint32_t connectedQueriesCount = 50000;

    std::string graphFileIn;

    for (int i = 1; i < argc; i++) {
        std::string content(argv[i]);

        if (content.length() > 2 && content[0] == '-' && content[1] == '-') {
            if (content == "--printStats") {
                printStats = true;
            } else if (content == "--allInOne") {
                allInOne = true;
            } else if (content == "--splitRandomFromConnected") {
                splitRandomFromConnected = true;
            } else if (content == "--graphFile") {
                if (i + 1 >= argc) {
                    std::cerr << "expected graph file name after --graphFile" << std::fatal;
                }

                if (!graphFileIn.empty()) {
                    std::cerr << "graph file name has already been set" << std::fatal;
                }

                graphFileIn = argv[++i];
            } else if (content == "--randomQueries") {
                if (i + 1 >= argc) {
                    std::cerr << "expected an integer after --randomQueries" << std::fatal;
                }

                std::string next(argv[++i]);
                randomQueriesCount = std::stoul(next);
            } else if (content == "--connectedQueries") {
                if (i + 1 >= argc) {
                    std::cerr << "expected an integer after --connectedQueries" << std::fatal;
                }

                std::string next(argv[++i]);
                connectedQueriesCount = std::stoul(next);
            } else {
                std::cerr << "unrecognized switch: " << content << std::endl;
                std::cerr << "Usage: --graphFile [graphFileIn] [--printStats] [--allInOne] "
                             "[--splitRandomFromConnected] --randomQueries [count] --connectedQueries [count]" << std::endl;
            }
        }
    }

    if (graphFileIn.empty()) {
        graphFileIn = { "./../workload/" + graphName + "/graph.nt" };
    }

    std::replace_if(graphFileIn.begin(), graphFileIn.end(), [](auto x) { return x == '\\'; }, '/');

    std::cout << "generating " << randomQueriesCount << " random queries and " << connectedQueriesCount
              << " connected queries \nfor graph: " << graphFileIn << std::endl << std::endl;

    auto labeledGraph = readGraph(graphFileIn);

    auto labelGenModes = createLabelGenModes(*labeledGraph);

    std::vector<QueryGenData> queryGenData;

    for (auto genMode : labelGenModes) {
        if (!splitRandomFromConnected) {
            QueryGenData genData;
            genData.name = "L" + std::to_string(genMode);
            genData.randomQueriesCount = randomQueriesCount;
            genData.connectedQueriesCount = connectedQueriesCount;
            genData.labelCount = genMode;

            queryGenData.push_back(genData);
        } else {
            QueryGenData genData;
            genData.name = "rnd.L" + std::to_string(genMode);
            genData.randomQueriesCount = randomQueriesCount;
            genData.connectedQueriesCount = 0;
            genData.labelCount = genMode;

            queryGenData.push_back(genData);

            QueryGenData genData2;
            genData2.name = "cnn.L" + std::to_string(genMode);
            genData2.randomQueriesCount = 0;
            genData2.connectedQueriesCount = connectedQueriesCount;
            genData2.labelCount = genMode;

            queryGenData.push_back(genData2);
        }
    }

    timer.begin("generate queries");
    std::unordered_map<std::string, std::vector<LCRQuery>> queries;
    generateQueries(*labeledGraph, queries, queryGenData);
    timer.end();

    if (allInOne) {
        auto mergedQueries = mergeQueries(*labeledGraph, queries, printStats);
        writeQueries(graphFileIn, mergedQueries);
    } else {
        auto splitQueries = evaluateQueries(*labeledGraph, queries, printStats);
        writeQueries(graphFileIn, splitQueries);
    }

    destroyThreadPool();
    return 0;
}