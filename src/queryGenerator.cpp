#include <io/GraphReader.hpp>
#include <generator/query/WalkerQueryGenerator.hpp>
#include <reachIndex/ReachabilityIndex.hpp>
#include "io/QueryWriter.hpp"
#include "utility/Timer.hpp"
#include "Selector.hpp"
#include "generator/query/WalkerStrategies.hpp"

Timer timer;

void queryStats(std::unique_ptr<SCCGraph> &sccGraph, const ReachQuerySet &queries) {
    auto truthIndex = ReachabilityIndex::create("bfs");

    truthIndex->setGraph(sccGraph.get());

    uint32_t trivialQueryCount = 0;
    std::vector<bool> truths(queries.size());

    for (auto i = 0u; i < queries.size(); i++) {
        auto &query = queries[i];

        auto sourceComponent = sccGraph->getComponentIndex(query.source);
        auto targetComponent = sccGraph->getComponentIndex(query.target);

        // A query is trivial when:
        // 1. The components are equal, since then it must be that the source and target are reachable
        // 2. The source is less then the target component, because the SCC graph is a topological sorted DAG.
        if (sourceComponent <= targetComponent) {
            trivialQueryCount++;
        }

        truths[i] = truthIndex->query(query);
    }

    auto totalTrue = std::accumulate(truths.begin(), truths.end(), 0);
    auto totalFalse = truths.size() - totalTrue;

    std::cout << "\nQuery stats: \n";
    std::cout << "total          = " << truths.size() << "\n";
    std::cout << "reachable      = " << totalTrue << "\n";
    std::cout << "not reachable  = " << totalFalse << "\n";
    std::cout << "trivial        = " << trivialQueryCount << "\n";
    std::cout << "used evaluator = " << truthIndex->getName() << "\n";
}

int main() {
    std::string graphFileIn { "./../workload/" + graphName + "/graph.nt" };
    std::string queryFileOut { "./../workload/" + graphName + "/queries.csv" };

    auto graphReader = GraphReader::createGraphReader();
    auto queryWriter = QueryWriter::createQueryWriter();

    timer.begin("reading graph");

    auto graph = graphReader->readGraph(graphFileIn);

    timer.end();

    timer.begin("creating scc graph");
    auto sccGraph = tarjanSCC(*graph, true);
    timer.end();

    std::cout << "\nSCC graph stats:\n" << *sccGraph << std::endl;
    std::cout << "\nSCC graph other stats: " << std::endl;
    printStats(std::cout, sccGraph->getComponentGraph());
    std::cout << std::endl;

    WalkerStartWithTopOutDegree startWithTopOutDegree(15);
    WalkerStartOnBestDegree startOnBestDegree(15);
    WalkerEmitTrueAndFalseQueries emitTrueAndFalseQueries;

    WalkerQueryGenerator queryGenerator(*sccGraph);
    startOnBestDegree.setup(queryGenerator);
    emitTrueAndFalseQueries.setup(queryGenerator);

    timer.begin("generate queries");

    auto queries = queryGenerator.generate(50000);

    timer.end();

    timer.begin("writing queries");
    queryWriter->writeQueries(*queries, queryFileOut);
    timer.end();

    queryStats(sccGraph, *queries);

    return 0;
}