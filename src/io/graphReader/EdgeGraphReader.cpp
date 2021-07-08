#include "EdgeGraphReader.hpp"

static void readCounts(const std::string &filePath, uint32_t &vertexCount, uint32_t &labelCount, uint32_t &edgeCount) {
    std::ifstream graphFile { filePath };

    std::string line;
    std::string_view lineView;
    size_t firstPart;
    size_t lastPart;

    labelCount = 0;
    vertexCount = 0;
    edgeCount = 0;

    while (std::getline(graphFile, line)) {
        lineView = line;

        firstPart = lineView.find_first_of(' ');
        lastPart = lineView.find_last_of(' ');

        Vertex source;
        Label label;
        Vertex target;

        source = std::stoul(lineView.data());
        target = std::stoul(lineView.data() + firstPart + 1);
        label = std::stoul(lineView.data() + lastPart + 1);

        vertexCount = std::max(vertexCount, std::max(source, target) + 1);
        labelCount = std::max(labelCount, label + 1);
        edgeCount++;
    }
}

static void readCountsAndFill(const std::string &filePath, std::vector<std::tuple<Vertex, Vertex, Label>> &edges,
                              uint32_t &vertexCount, uint32_t &labelCount) {
    std::ifstream graphFile { filePath };

    std::string line;
    std::string_view lineView;
    size_t firstPart;
    size_t lastPart;

    labelCount = 0;
    vertexCount = 0;

    while (std::getline(graphFile, line)) {
        lineView = line;

        firstPart = lineView.find_first_of(' ');
        lastPart = lineView.find_last_of(' ');

        Vertex source;
        Label label;
        Vertex target;

        source = std::stoul(lineView.data());
        target = std::stoul(lineView.data() + firstPart + 1);
        label = std::stoul(lineView.data() + lastPart + 1);

        vertexCount = std::max(vertexCount, std::max(source, target) + 1);
        labelCount = std::max(labelCount, label + 1);

        edges.emplace_back(source, label, target);
    }
}

std::unique_ptr<DiGraph> EdgeGraphReader::readGraph(const std::string &filePath) {
    std::ifstream graphFile { filePath };

    std::string line;
    std::string_view lineView;
    size_t firstPart;
    size_t lastPart;

    uint32_t vertexCount;
    uint32_t labelCount;
    uint32_t edgeCount;

    readCounts(filePath, vertexCount, labelCount, edgeCount);

    auto graph = std::make_unique<DiGraph>();
    graph->setVertices(vertexCount);

    // parse edge data
    while (std::getline(graphFile, line)) {
        lineView = line;

        firstPart = lineView.find_first_of(' ');
        lastPart = lineView.find_last_of(' ');

        Vertex source;
        Vertex target;

        source = std::stoul(lineView.data());
        target = std::stoul(lineView.data() + firstPart + 1);

        graph->addEdge(source, target);
    }

    return graph;
}

std::unique_ptr<LabeledGraph> EdgeGraphReader::readLabeledGraph(const std::string &filePath) {
    std::vector<std::tuple<Vertex, Vertex, Label>> edges;

    uint32_t vertexCount = 0;
    uint32_t labelCount = 0;

    readCountsAndFill(filePath, edges, vertexCount, labelCount);
    return createLabeledGraph(edges, vertexCount, labelCount);
}

std::unique_ptr<LabeledEdgeGraph> EdgeGraphReader::readLabeledEdgeGraph(const std::string &filePath) {
    std::ifstream graphFile { filePath };

    std::string line;
    std::string_view lineView;
    size_t firstPart;
    size_t lastPart;

    uint32_t vertexCount;
    uint32_t labelCount;
    uint32_t edgeCount;

    readCounts(filePath, vertexCount, labelCount, edgeCount);

    auto graph = std::make_unique<LabeledEdgeGraph>();
    graph->setSizes(vertexCount, labelCount, edgeCount);

    // parse edge data
    while (std::getline(graphFile, line)) {
        lineView = line;

        firstPart = lineView.find_first_of(' ');
        lastPart = lineView.find_last_of(' ');

        Vertex source;
        Label label;
        Vertex target;

        source = std::stoul(lineView.data());
        target = std::stoul(lineView.data() + firstPart + 1);
        label = std::stoul(lineView.data() + lastPart + 1);

        graph->addEdge(source, target, label);
    }

    return graph;
}

std::unique_ptr<PerLabelGraph> EdgeGraphReader::readPerLabelGraph(const std::string &filePath) {
    std::ifstream graphFile { filePath };

    std::string line;
    std::string_view lineView;
    size_t firstPart;
    size_t lastPart;

    uint32_t vertexCount;
    uint32_t labelCount;
    uint32_t edgeCount;

    readCounts(filePath, vertexCount, labelCount, edgeCount);

    auto graph = std::make_unique<PerLabelGraph>();
    graph->setSizes(vertexCount, labelCount);

    // parse edge data
    while (std::getline(graphFile, line)) {
        lineView = line;

        firstPart = lineView.find_first_of(' ');
        lastPart = lineView.find_last_of(' ');

        Vertex source;
        Label label;
        Vertex target;

        source = std::stoul(lineView.data());
        target = std::stoul(lineView.data() + firstPart + 1);
        label = std::stoul(lineView.data() + lastPart + 1);

        graph->addEdge(source, target, label);
    }

    return graph;
}
