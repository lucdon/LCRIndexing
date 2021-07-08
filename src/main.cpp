#include <evaluation/ReachQueriesRunner.hpp>
#include <evaluation/LCRQueriesRunner.hpp>
#include "Selector.hpp"

void readArgsAndRun(int argc, char *const *argv, bool doReachQueries);
void runCustom(bool doReachQueries);

int main(int argc, char **argv) {
    if (argc <= 1) {
        std::cerr << "Usage: [reach|lcr] --graphFile [graphFile] --queryFile [queriesFile]"
                     " --index [indexName] --indexParams [parameterList]"
                     " [--control] --timeLimit [timeLimitInSeconds] --memoryLimit [memoryLimitInMBs]" << std::endl;
        return 1;
    }

    std::string runMode = std::string(argv[1]);
    std::string runModeLowercase;
    runModeLowercase.resize(runMode.size());
    std::transform(runMode.begin(), runMode.end(), runModeLowercase.begin(), ::tolower);

    bool doReachQueries;

    if (runModeLowercase == "reach") {
        doReachQueries = true;
    } else if (runModeLowercase == "lcr") {
        doReachQueries = false;
    } else {
        std::cerr << "expected either reach or lcr as first argument!" << std::endl;
        return 1;
    }

    if (argc == 2) {
        runCustom(doReachQueries);
    } else {
        readArgsAndRun(argc, argv, doReachQueries);
    }

    return 0;
}

void runCustom(bool doReachQueries) {
    if (doReachQueries) {
        std::vector<std::string> queryFiles;
        queryFiles.emplace_back("./../workload/" + graphName + "/queries.csv");
        std::string graphFileIn { "./../workload/" + graphName + "/graph.nt" };

        ReachQueriesRunner runner;

        auto multiLimit = std::make_unique<MultiLimit>();
        multiLimit->addLimit(std::make_unique<TimeLimit>(std::chrono::hours(1)));
        multiLimit->addLimit(std::make_unique<MemoryLimit>(25 * 1000));
        runner.setLimit(std::move(multiLimit));

        //runner.setControlIndex(ReachabilityIndex::create("bfs"));

        // Set of very basic options.
        //runner.addIndex(ReachabilityIndex::create("BFS"));
        //runner.addIndex(ReachabilityIndex::create("Hybrid-BFS"));
        //runner.addIndex(ReachabilityIndex::create("DFS"));
        //runner.addIndex(ReachabilityIndex::create("BiBFS"));
        //runner.addIndex(ReachabilityIndex::create("TC"));

        // Pruned landmark labeling based indices.
        //runner.addIndex(ReachabilityIndex::create("PLL"));
        //runner.addIndex(ReachabilityIndex::create("Heatmap-PLL"));

        // Pruned path labeling based indices.
        //runner.addIndex(ReachabilityIndex::create("PPL"));
        //runner.addIndex(ReachabilityIndex::create("Heatmap-PPL"));

        // Bloom filter labeling based indices.
        //runner.addIndex(ReachabilityIndex::create("BFL", "5"));
        //runner.addIndex(ReachabilityIndex::create("BFL", "10"));

        runner.run(graphFileIn, queryFiles);
    } else {
        std::vector<std::string> queryFiles;

        std::string graphFileIn { "./../workload/generated/erV500kD3L4096exp.nt" };
        //queryFiles.emplace_back("./../workload/generated/erV25kD3L64exp.queries-lcr.L2.false.csv");
        //queryFiles.emplace_back("./../workload/generated/erV25kD3L64exp.queries-lcr.L2.true.csv");
        queryFiles.emplace_back("./../workload/generated/erV500kD3L4096exp.queries-lcr.L4.false.csv");
        queryFiles.emplace_back("./../workload/generated/erV25kD3L64exp.queries-lcr.L4.true.csv");
        //queryFiles.emplace_back("./../workload/generated/erV25kD3L64exp.queries-lcr.L6.false.csv");
        //queryFiles.emplace_back("./../workload/generated/erV25kD3L64exp.queries-lcr.L6.true.csv");

        //std::string graphFileIn { "./../workload/" + graphName + "/graph.nt" };
        //queryFiles.emplace_back("./../workload/" + graphName + "/graph.queries-lcr.cnn.L1.false.csv");
        //queryFiles.emplace_back("./../workload/" + graphName + "/graph.queries-lcr.cnn.L1.true.csv");
        //queryFiles.emplace_back("./../workload/" + graphName + "/graph.queries-lcr.rnd.L1.false.csv");
        //queryFiles.emplace_back("./../workload/" + graphName + "/graph.queries-lcr.rnd.L1.true.csv");

        //queryFiles.emplace_back("./../workload/" + graphName + "/graph.queries-lcr.cnn.L4.false.csv");
        //queryFiles.emplace_back("./../workload/" + graphName + "/graph.queries-lcr.cnn.L4.true.csv");
        //queryFiles.emplace_back("./../workload/" + graphName + "/graph.queries-lcr.rnd.L4.false.csv");
        //queryFiles.emplace_back("./../workload/" + graphName + "/graph.queries-lcr.rnd.L4.true.csv");

        lcr::QueriesRunner runner;

        //auto multiLimit = std::make_unique<MultiLimit>();
        //multiLimit->addLimit(std::make_unique<TimeLimit>(std::chrono::hours(1)));
        //multiLimit->addLimit(std::make_unique<MemoryLimit>(25 * 1000));
        //runner.setLimit(std::move(multiLimit));

        //runner.setControlIndex(lcr::Index::create("bfs"));

        // Non-scaling options
        //runner.addIndex(lcr::Index::create("bfs"));
        //runner.addIndex(lcr::Index::create("ALC", "PLL"));
        //runner.addIndex(lcr::Index::create("KLC", "PLL"));
        //runner.addIndex(lcr::Index::create("KLC-BFL"));
        //runner.addIndex(lcr::Index::create("KLC-Freq", "PLL"));
        runner.addIndex(lcr::Index::create("pruned-2-hop"));
        //runner.addIndex(lcr::Index::create("li+", "custom", "20"));
        //runner.addIndex(lcr::Index::create("LWBF", "custom", "custom", "32"));

        // Scaling options
        //runner.addIndex(lcr::Index::create("sh", "klc", "pll"));
        //runner.addIndex(lcr::Index::create("sh", "KLC-Freq", "pll"));
        //runner.addIndex(lcr::Index::create("sh", "KLC-BFL"));
        runner.addIndex(lcr::Index::create("sh", "pruned-2-hop"));
        //runner.addIndex(lcr::Index::create("sh", "li+", "custom", "20"));
        //runner.addIndex(lcr::Index::create("sh", "LWBF", "custom", "custom", "32"));

        // Not working options
        //runner.addIndex(lcr::Index::create("BFL-path", "4"));
        runner.addIndex(lcr::Index::create("sh", "BFL-path", "4"));
        //runner.addIndex(lcr::Index::create("bgi", "4"));
        //runner.addIndex(lcr::Index::create("bpi", "4"));
        //runner.addIndex(lcr::Index::create("bfi", "4"));
        //runner.addIndex(lcr::Index::create("bgmi", "4"));

        runner.run(graphFileIn, queryFiles);
    }
}

void readArgsAndRun(int argc, char *const *argv, bool doReachQueries) {
    std::string index;
    std::string graphFile;
    std::vector<std::string> queryFiles;
    bool control = false;
    std::vector<std::string> indexParams;

    int64_t timeLimit = -1;
    int64_t memoryLimit = -1;

    for (int i = 2; i < argc; i++) {
        std::string content(argv[i]);

        if (content.length() > 2 && content[0] == '-' && content[1] == '-') {
            if (content == "--index") {
                if (i + 1 >= argc) {
                    std::cerr << "expected index name after --index" << std::fatal;
                }

                if (!index.empty()) {
                    std::cerr << "index has already been set" << std::fatal;
                }

                index = argv[++i];
            } else if (content == "--indexParams") {
                if (!indexParams.empty()) {
                    std::cerr << "indexParams has already been set" << std::fatal;
                }

                while (i + 1 < argc) {
                    std::string next(argv[i + 1]);

                    if (next.length() > 2 && next[0] == '-' && next[1] == '-') {
                        break;
                    }

                    indexParams.emplace_back(next);

                    i++;
                }
            } else if (content == "--graphFile") {
                if (i + 1 >= argc) {
                    std::cerr << "expected graph file name after --graphFile" << std::fatal;
                }

                if (!graphFile.empty()) {
                    std::cerr << "graph file name has already been set" << std::fatal;
                }

                graphFile = argv[++i];
            } else if (content == "--queryFile") {
                if (i + 1 >= argc) {
                    std::cerr << "expected query file name after --queryFile" << std::fatal;
                }

                queryFiles.emplace_back(argv[++i]);
            } else if (content == "--timeLimit") {
                if (i + 1 >= argc) {
                    std::cerr << "expected timeLimit after --timeLimit" << std::fatal;
                }

                std::string next(argv[++i]);
                timeLimit = std::stoll(next);
            } else if (content == "--memoryLimit") {
                if (i + 1 >= argc) {
                    std::cerr << "expected timeLimit after --memoryLimit" << std::fatal;
                }

                std::string next(argv[++i]);
                memoryLimit = std::stoll(next);
            } else if (content == "--control") {
                control = true;
            } else {
                std::cerr << "unrecognized switch: " << content << std::fatal;
            }
        }
    }

    if (graphFile.empty()) {
        std::cerr << "--graphFile [file] is required!" << std::fatal;
    }

    if (queryFiles.empty()) {
        std::cerr << "--queryFile [file] is required!" << std::fatal;
    }

    if (index.empty()) {
        std::cerr << "--index [indexName] is required!" << std::fatal;
    }

    if (doReachQueries) {
        ReachQueriesRunner runner;

        auto multiLimit = std::make_unique<MultiLimit>();

        if (timeLimit > 0) {
            multiLimit->addLimit(std::make_unique<TimeLimit>(std::chrono::seconds(timeLimit)));
        }

        if (memoryLimit > 0) {
            multiLimit->addLimit(std::make_unique<MemoryLimit>(uint32_t(memoryLimit)));
        }

        if (control) {
            runner.setControlIndex(ReachabilityIndex::create("BFS"));
        }

        runner.setLimit(std::move(multiLimit));
        runner.addIndex(ReachabilityIndex::create(index, indexParams));
        runner.run(graphFile, queryFiles);
    } else {
        lcr::QueriesRunner runner;

        auto multiLimit = std::make_unique<MultiLimit>();

        if (timeLimit > 0) {
            multiLimit->addLimit(std::make_unique<TimeLimit>(std::chrono::seconds(timeLimit)));
        }

        if (memoryLimit > 0) {
            multiLimit->addLimit(std::make_unique<MemoryLimit>(uint32_t(memoryLimit)));
        }

        if (control) {
            runner.setControlIndex(lcr::Index::create("BFS"));
        }

        runner.setLimit(std::move(multiLimit));
        runner.addIndex(lcr::Index::create(index, indexParams));
        runner.run(graphFile, queryFiles);
    }
}
