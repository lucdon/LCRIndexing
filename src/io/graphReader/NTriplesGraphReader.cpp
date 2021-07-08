#include "NTriplesGraphReader.hpp"

std::unique_ptr<DiGraph> NTriplesGraphReader::readGraph(const std::string &filePath) {
    std::string line;
    std::ifstream graphFile { filePath };

    std::getline(graphFile, line);
    std::string_view lineView = line;

    auto firstPart = lineView.find_first_of(',');
    auto lastPart = lineView.find_last_of(',');

    auto graph = std::make_unique<DiGraph>();

    if (firstPart != std::string_view::npos && lastPart != std::string_view::npos && firstPart != lastPart) {
        uint32_t noNodes = std::stoul(lineView.data());
        graph->setVertices(noNodes);
    } else {
        std::cerr << "Invalid graph header!" << std::fatal;
    }

    // parse edge data
    while (std::getline(graphFile, line)) {
        lineView = line;

        firstPart = lineView.find_first_of(' ');
        lastPart = lineView.find_last_of(' ');

        lineView = lineView.substr(0, lastPart);
        lastPart = lineView.find_last_of(' ');

        Vertex source;
        Vertex target;

        source = std::stoul(lineView.data());
        target = std::stoul(lineView.data() + lastPart + 1);

        graph->addEdge(source, target);
    }

    return graph;
}

std::unique_ptr<LabeledEdgeGraph> NTriplesGraphReader::readLabeledEdgeGraph(const std::string &filePath) {
    std::string line;
    std::ifstream graphFile { filePath };

    std::getline(graphFile, line);
    std::string_view lineView = line;

    auto firstPart = lineView.find_first_of(',');
    auto lastPart = lineView.find_last_of(',');

    uint32_t vertexCount = 0;
    uint32_t edgeCount = 0;
    uint32_t labelCount = 0;

    if (firstPart != std::string_view::npos && lastPart != std::string_view::npos && firstPart != lastPart) {
        vertexCount = std::stoul(lineView.data());
        edgeCount = std::stoul(lineView.data() + firstPart + 1);
        labelCount = std::stoul(lineView.data() + lastPart + 1);
    } else {
        std::cerr << "Invalid graph header!" << std::fatal;
    }

    auto labeledGraph = std::make_unique<LabeledEdgeGraph>();
    labeledGraph->setSizes(vertexCount, labelCount, edgeCount);

    // parse edge data
    while (std::getline(graphFile, line)) {
        lineView = line;

        firstPart = lineView.find_first_of(' ');
        lastPart = lineView.find_last_of(' ');

        lineView = lineView.substr(0, lastPart);
        lastPart = lineView.find_last_of(' ');

        Vertex source;
        uint32_t label;
        Vertex target;

        source = std::stoul(lineView.data());
        label = std::stoul(lineView.data() + firstPart + 1);
        target = std::stoul(lineView.data() + lastPart + 1);

        labeledGraph->addEdge(source, target, label);
    }

    return labeledGraph;
}

void
readSizesAndFill(const std::string &filePath, std::vector<std::tuple<Vertex, Vertex, Label>> &edges, uint32_t &vertices,
                 uint32_t &labels) {
    std::string line;
    std::ifstream graphFile { filePath };

    std::getline(graphFile, line);
    std::string_view lineView = line;

    auto firstPart = lineView.find_first_of(',');
    auto lastPart = lineView.find_last_of(',');

    uint32_t vertexCount = 0;
    uint32_t edgeCount = 0;
    uint32_t labelCount = 0;

    if (firstPart != std::string_view::npos && lastPart != std::string_view::npos && firstPart != lastPart) {
        vertexCount = std::stoul(lineView.data());
        edgeCount = std::stoul(lineView.data() + firstPart + 1);
        labelCount = std::stoul(lineView.data() + lastPart + 1);
    } else {
        std::cerr << "Invalid graph header!" << std::fatal;
    }

    edges.reserve(edgeCount);
    vertices = vertexCount;
    labels = labelCount;

    // parse edge data
    while (std::getline(graphFile, line)) {
        lineView = line;

        firstPart = lineView.find_first_of(' ');
        lastPart = lineView.find_last_of(' ');

        lineView = lineView.substr(0, lastPart);
        lastPart = lineView.find_last_of(' ');

        Vertex source;
        uint32_t label;
        Vertex target;

        source = std::stoul(lineView.data());
        label = std::stoul(lineView.data() + firstPart + 1);
        target = std::stoul(lineView.data() + lastPart + 1);

        edges.emplace_back(source, target, label);
    }
}

std::unique_ptr<LabeledGraph> NTriplesGraphReader::readLabeledGraph(const std::string &filePath) {
    std::vector<std::tuple<Vertex, Vertex, Label>> edges;

    uint32_t vertexCount = 0;
    uint32_t labelCount = 0;

    readSizesAndFill(filePath, edges, vertexCount, labelCount);
    return createLabeledGraph(edges, vertexCount, labelCount);
}

std::unique_ptr<PerLabelGraph> NTriplesGraphReader::readPerLabelGraph(const std::string &filePath) {
    std::string line;
    std::ifstream graphFile { filePath };

    std::getline(graphFile, line);
    std::string_view lineView = line;

    auto firstPart = lineView.find_first_of(',');
    auto lastPart = lineView.find_last_of(',');

    uint32_t vertexCount = 0;
    uint32_t edgeCount = 0;
    uint32_t labelCount = 0;

    if (firstPart != std::string_view::npos && lastPart != std::string_view::npos && firstPart != lastPart) {
        vertexCount = std::stoul(lineView.data());
        edgeCount = std::stoul(lineView.data() + firstPart + 1);
        labelCount = std::stoul(lineView.data() + lastPart + 1);
    } else {
        std::cerr << "Invalid graph header!" << std::fatal;
    }

    auto labeledGraph = std::make_unique<PerLabelGraph>();
    labeledGraph->setSizes(vertexCount, labelCount);

    // parse edge data
    while (std::getline(graphFile, line)) {
        lineView = line;

        firstPart = lineView.find_first_of(' ');
        lastPart = lineView.find_last_of(' ');

        lineView = lineView.substr(0, lastPart);
        lastPart = lineView.find_last_of(' ');

        Vertex source;
        uint32_t label;
        Vertex target;

        source = std::stoul(lineView.data());
        label = std::stoul(lineView.data() + firstPart + 1);
        target = std::stoul(lineView.data() + lastPart + 1);

        labeledGraph->addEdge(source, target, label);
    }

    return labeledGraph;
}
