#include <graphs/Query.hpp>
#include <utility/MemoryWatch.hpp>
#include <utility/Timer.hpp>
#include <utility/StepTimer.hpp>
#include <io/QueryReader.hpp>
#include <io/GraphReader.hpp>
#include <utility/CategorizedStepTimer.hpp>
#include "LCRQueriesRunner.hpp"

namespace lcr {
    static Timer timer;
    static MemoryWatch memoryWatch;

    void train(const std::vector<std::unique_ptr<Index>> &indices) {
        std::cout << "\nTraining timings:" << std::endl;

        for (auto &index : indices) {
            memoryWatch.begin();
            timer.begin(index->getName());
            index->train();
            timer.endSameLine();
            memoryWatch.endSameLine();
            std::cout << std::endl;
        }
    }

    void printStats(const std::vector<std::unique_ptr<Index>> &indices) {
        std::cout << "\nIndexes used:" << std::endl;

        for (auto &index : indices) {
            std::cout << *index << std::endl;
        }
    }

    void test(std::unique_ptr<Index> &index, const LCRQuery &lcrQuery, bool truth) {
        auto result = index->query(lcrQuery);

        if (result == truth) {
            return;
        }

        std::cout << "    query: " << lcrQuery << " result: " << (result ? "true" : "false") << " expected: "
                  << (truth ? "true" : "false") << "\n";
    }

    void query(std::vector<std::unique_ptr<Index>> &indices, const std::string &fileName, const LCRQuerySet &queries) {
        std::cout << "\nQuery timings: " << fileName << std::endl;

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

    void query(Index &truthIndex, std::vector<std::unique_ptr<Index>> &indices, const std::string &fileName,
               LCRQuerySet &queries) {
        boost::dynamic_bitset<> truths(queries.size());

        for (auto i = 0u; i < queries.size(); i++) {
            auto &query = queries[i];

            truths[i] = truthIndex.query(query);
        }

        auto totalTrue = truths.count();
        auto totalFalse = truths.size() - totalTrue;

        std::cout << "\nQuery stats: " << fileName << "\n";
        std::cout << "total          = " << truths.size() << "\n";
        std::cout << "reachable      = " << totalTrue << "\n";
        std::cout << "not reachable  = " << totalFalse << "\n";
        std::cout << "used evaluator = " << truthIndex.getName() << "\n";

        std::cout << "\nQuery timings: " << std::endl;
        CategorizedStepTimer stepTimer;

        auto allCategory = 0;
        auto labelCategoryOffset = 10;

        stepTimer.setIndent(4);
        stepTimer.addCategory(allCategory, "all");
        stepTimer.addCategory(1, "false queries");
        stepTimer.addCategory(2, "true queries");

        for (auto &index : indices) {
            for (auto i = 0u; i < queries.size(); i++) {
                uint32_t truthCat = uint32_t(truths[i]) + 1;
                uint32_t labelCat = queries[i].labelSet.count();

                if (!stepTimer.hasCategory(labelCat + labelCategoryOffset)) {
                    stepTimer.addCategory(labelCat + labelCategoryOffset,
                                          std::string("labelCount: ") + std::to_string(labelCat));
                }

                stepTimer.beginStep();
                test(index, queries[i], truths[i]);
                stepTimer.endStep(allCategory, truthCat, labelCat + labelCategoryOffset);
            }

            formatWidth(std::cout, index->getName(), 50);
            std::cout << std::endl << stepTimer << std::endl;
            stepTimer.reset();
        }
    }

    std::unique_ptr<LCRQuerySet> readQueries(const std::string &queryFile) {
        auto queryReader = QueryReader::createQueryReader();
        auto queries = queryReader->readLabeledQueries(queryFile);
        return queries;
    }

    std::unique_ptr<LabeledEdgeGraph> readEdgeGraph(const std::string &graphFile) {
        auto graphReader = GraphReader::createGraphReader();

        memoryWatch.begin();
        timer.begin("Read graph");
        auto graph = graphReader->readLabeledEdgeGraph(graphFile);
        timer.endSameLine();
        memoryWatch.end();

        return graph;
    }

    void QueriesRunner::run(std::string &graphFile, const std::vector<std::string> &queryFiles) {
        LimitRunner limitRunner(limit);
        limitRunner.start();

        bool hasControl = controlIndex != nullptr;

        auto graph = readEdgeGraph(graphFile);

        if (hasControl) {
            std::cout << "\nLabeled graph stats:\n" << *graph << std::endl << std::endl;
            printLabelDistribution(std::cout, *graph);
            printVertexDistribution(std::cout, *graph);
        } else {
            std::cout << std::endl;
        }

        if (hasControl) {
            controlIndex->setGraph(graph.get());
        }

        for (auto &index : indices) {
            index->setGraph(graph.get());
        }

        std::cout << "Registered indexes: " << std::endl;

        for (auto &index : indices) {
            formatWidth(std::cout, index->getName(), 50);
            std::cout << std::endl;
        }

        if (hasControl) {
            controlIndex->train();
        }

        train(indices);

        std::vector<std::pair<std::string, std::shared_ptr<LCRQuerySet>>> querySets;

        for (auto &queryFile : queryFiles) {
            auto queries = readQueries(queryFile);

            for (auto &query : *queries) {
                query.init(*graph);
            }

            querySets.emplace_back(queryFile, std::move(queries));
        }

        printStats(indices);

        for (auto &queryPair : querySets) {
            if (hasControl) {
                query(*controlIndex, indices, queryPair.first, *queryPair.second);
            } else {
                query(indices, queryPair.first, *queryPair.second);
            }
        }

        limitRunner.stop();
    }
}