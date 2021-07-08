#include <io/GraphReader.hpp>
#include <graphs/SCCGraph.hpp>
#include "io/GraphWriter.hpp"
#include "utility/Timer.hpp"
#include "Selector.hpp"

Timer timer;

void printGraphStats(LabeledEdgeGraph &graph) {
    std::cout << "\nLabeled graph stats:\n" << graph << std::endl;

    std::cout << "Avg Degree: " << (double(graph.getEdgeCount()) / double(graph.getVertexCount())) << std::endl
              << std::endl;

    printVertexDistribution(std::cout, graph);
    printLabelDistribution(std::cout, graph);

    auto sccGraph = tarjanSCC(graph, true);

    printComponentDistribution(std::cout, *sccGraph);
}

int main(int argc, char **argv) {
    if (argc < 1) {
        std::cerr << "Usage: --graphFile [graphFileIn] [--printStats] [--graphFileOut] [graphFileOut] " << std::endl;
        return 1;
    }

    bool printStats = false;

    std::string graphFileIn;
    std::string graphFileOut;

    for (int i = 1; i < argc; i++) {
        std::string content(argv[i]);

        if (content.length() > 2 && content[0] == '-' && content[1] == '-') {
            if (content == "--printStats") {
                printStats = true;
            } else if (content == "--graphFile") {
                if (i + 1 >= argc) {
                    std::cerr << "expected graph file name after --graphFile" << std::fatal;
                }

                if (!graphFileIn.empty()) {
                    std::cerr << "graph file name has already been set" << std::fatal;
                }

                graphFileIn = argv[++i];
            } else if (content == "--graphFileOut") {
                if (i + 1 >= argc) {
                    std::cerr << "expected graph file name after --graphFileOut" << std::fatal;
                }

                if (!graphFileOut.empty()) {
                    std::cerr << "graph file name has already been set" << std::fatal;
                }

                graphFileOut = argv[++i];
            } else {
                std::cerr << "unrecognized switch: " << content << std::endl;
                std::cerr << "Usage: --graphFile [graphFileIn] [--printStats] [--graphFileOut] [graphFileOut] "
                          << std::endl;
            }
        }
    }

    if (graphFileIn.empty()) {
        //graphFileIn = { "./../workload/" + graphName + "/graph" + graphFileInExtension };
        graphFileIn =  "./../workload/generated/plV500Ka1.95L64exp.nt" ;
        //graphFileIn =  "./../workload/generated/ffV100k3L64exp.nt" ;
        //graphFileIn =  "./../workload/generated/erV100kD3L64exp.nt" ;
    }

    auto graphReader = GraphReader::createGraphReader();

    timer.begin("reading graph");
    auto graph = graphReader->readLabeledEdgeGraph(graphFileIn);
    timer.end();

    if (printStats) {
        printGraphStats(*graph);
    }

    auto graphWriter = GraphWriter::createGraphWriter();

    if (!graphFileOut.empty()) {
        timer.begin("writing graph");
        graphWriter->writeLabeledGraph(*graph, graphFileOut);
        timer.end();
    }

    return 0;
}
