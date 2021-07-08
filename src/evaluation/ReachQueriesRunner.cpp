#include <graphs/Query.hpp>
#include <utility/MemoryWatch.hpp>
#include <utility/Timer.hpp>
#include <utility/StepTimer.hpp>
#include <reachIndex/ReachabilityIndex.hpp>
#include <io/QueryReader.hpp>
#include <io/GraphReader.hpp>
#include "ReachQueriesRunner.hpp"

static Timer timer;
static MemoryWatch memoryWatch;

void train(const std::vector<std::unique_ptr<ReachabilityIndex>> &indices) {
    std::cout << "\nTraining timings: \n";
    for (auto &index : indices) {
        memoryWatch.begin();
        timer.begin(index->getName());
        index->train();
        timer.endSameLine();
        memoryWatch.endSameLine();
        std::cout << std::endl;
    }
}

void printStats(const std::vector<std::unique_ptr<ReachabilityIndex>> &indices) {
    std::cout << "\nIndexes used: \n";

    for (auto &index : indices) {
        std::cout << *index << std::endl;
    }
}

void test(std::unique_ptr<ReachabilityIndex> &index, const ReachQuery &reachQuery, bool truth) {
    auto result = index->query(reachQuery);

    if (result == truth) {
        return;
    }

    std::cout << "    query: " << reachQuery << " result: " << (result ? "true" : "false") << " expected: "
              << (truth ? "true" : "false") << "\n";
}

void query(const SCCGraph &sccGraph, std::vector<std::unique_ptr<ReachabilityIndex>> &indices,
           const ReachQuerySet &queries) {
    std::cout << "\nQuery timings: " << std::endl;

    for (auto &index : indices) {
        StepTimer stepTimer;

        for (const auto &query : queries) {
            stepTimer.beginStep();
            index->query(query);
            stepTimer.endStep();
        }

        formatWidth(std::cout, index->getName(), 50);
        std::cout << stepTimer << std::endl;
    }
}

void
query(const SCCGraph &sccGraph, ReachabilityIndex &truthIndex, std::vector<std::unique_ptr<ReachabilityIndex>> &indices,
      const ReachQuerySet &queries) {

    uint32_t trivialQueryCount = 0;
    std::vector<bool> truths(queries.size());

    for (auto i = 0u; i < queries.size(); i++) {
        auto &query = queries[i];

        auto sourceComponent = sccGraph.getComponentIndex(query.source);
        auto targetComponent = sccGraph.getComponentIndex(query.target);

        // A query is trivial when:
        // 1. The components are equal, since then it must be that the source and target are reachable
        // 2. The source is less then the target component, because the SCC graph is a topological sorted DAG.
        if (sourceComponent <= targetComponent) {
            trivialQueryCount++;
        }

        truths[i] = truthIndex.query(query);
    }

    auto totalTrue = std::accumulate(truths.begin(), truths.end(), 0);
    auto totalFalse = truths.size() - totalTrue;

    std::cout << "\nQuery stats: \n";
    std::cout << "total          = " << truths.size() << "\n";
    std::cout << "reachable      = " << totalTrue << "\n";
    std::cout << "not reachable  = " << totalFalse << "\n";
    std::cout << "trivial        = " << trivialQueryCount << "\n";
    std::cout << "used evaluator = " << truthIndex.getName() << "\n";

    std::cout << "\nQuery timings: " << std::endl;

    for (auto &index : indices) {
        StepTimer trueStepTimer;
        StepTimer falseStepTimer;

        for (auto i = 0u; i < queries.size(); i++) {
            if (truths[i]) {
                trueStepTimer.beginStep();
                test(index, queries[i], truths[i]);
                trueStepTimer.endStep();
            } else {
                falseStepTimer.beginStep();
                test(index, queries[i], truths[i]);
                falseStepTimer.endStep();
            }
        }

        formatWidth(std::cout, index->getName() + " reachable", 50);
        std::cout << trueStepTimer << std::endl;

        formatWidth(std::cout, index->getName() + " not reachable", 50);
        std::cout << falseStepTimer << std::endl;
    }
}

std::unique_ptr<std::vector<ReachQuery>> readQueries(const std::string &queryFile) {
    auto queryReader = QueryReader::createQueryReader();
    auto queries = queryReader->readQueries(queryFile);
    return queries;
}

std::unique_ptr<SCCGraph> readLCRGraph(const std::string &graphFile) {

    auto graphReader = GraphReader::createGraphReader();

    memoryWatch.begin();
    timer.begin("Read graph");
    auto graph = graphReader->readGraph(graphFile);
    timer.endSameLine();
    memoryWatch.end();

    std::cout << std::endl;

    memoryWatch.begin();
    timer.begin("Create SCC Graph");
    auto sccGraph = tarjanSCC(*graph, true);
    timer.endSameLine();
    memoryWatch.end();

    std::cout << "\nSCC graph stats:\n" << *sccGraph << std::endl;

    return sccGraph;
}

void ReachQueriesRunner::run(std::string &graphFile, const std::vector<std::string> &queryFiles) {
    LimitRunner limitRunner(limit);
    limitRunner.start();

    std::cout << "Registered indexes: " << std::endl;

    for (auto &index : indices) {
        formatWidth(std::cout, index->getName(), 50);
        std::cout << std::endl;
    }

    std::cout << std::endl;

    bool hasControl = controlIndex != nullptr;
    auto sccGraph = readLCRGraph(graphFile);

    if (hasControl) {
        controlIndex->setGraph(sccGraph.get());
    }

    for (auto &index : indices) {
        index->setGraph(sccGraph.get());
    }

    train(indices);
    printStats(indices);

    for(auto& queryFile : queryFiles) {
        auto queries = readQueries(queryFile);

        if (hasControl) {
            query(*sccGraph, *controlIndex, indices, *queries);
        } else {
            query(*sccGraph, indices, *queries);
        }
    }

    limitRunner.stop();
}
