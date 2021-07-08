#include "RdfGraphReader.hpp"

std::unique_ptr<DiGraph> RdfGraphReader::readGraph(const std::string &filePath) {
    std::string line;
    std::ifstream graphFile { filePath };

    std::string_view lineView;
    size_t index;

    std::string_view subject;
    std::string_view object;

    uint32_t currentIndex = 0;
    std::unordered_map<std::string, Vertex> labelToIndex;
    std::vector<std::pair<Vertex, Vertex>> edges;

    // parse edge data
    while (std::getline(graphFile, line)) {
        lineView = line;

        if (lineView[0] == '\"') {
            // Find closing.
            index = lineView.find_first_of('\"', 1);
            subject = lineView.substr(0, index + 1);
            lineView = lineView.substr(index + 2);
        } else {
            index = lineView.find_first_of(' ');
            subject = lineView.substr(0, index);
            lineView = lineView.substr(index + 1);
        }

        // For now skip predicate.
        if (lineView[0] == '\"') {
            // Find closing.
            index = lineView.find_first_of('\"', 1);
            lineView = lineView.substr(index + 2);
        } else {
            index = lineView.find_first_of(' ');
            lineView = lineView.substr(index + 1);
        }

        if (lineView[0] == '\"') {
            // Find closing.
            index = lineView.find_first_of('\"', 1);
            object = lineView.substr(0, index + 1);
        } else {
            index = lineView.find_first_of(' ');
            object = lineView.substr(0, index);
        }

        std::string subjectStr = { subject.data(), subject.size() };
        std::string objectStr = { object.data(), object.size() };

        if (labelToIndex.count(subjectStr) == 0) {
            labelToIndex.emplace(subjectStr, currentIndex++);
        }

        if (labelToIndex.count(objectStr) == 0) {
            labelToIndex.emplace(objectStr, currentIndex++);
        }

        auto subjectIdx = labelToIndex[subjectStr];
        auto objectIdx = labelToIndex[objectStr];

        edges.emplace_back(subjectIdx, objectIdx);
    }

    // Construct the graph.
    auto graph = std::make_unique<DiGraph>();
    graph->setVertices(currentIndex);

    for (auto &edge : edges) {
        graph->addEdge(edge.first, edge.second);
    }

    return graph;
}

std::unique_ptr<LabeledGraph> RdfGraphReader::readLabeledGraph(const std::string &filePath) {
    std::string line;
    std::ifstream graphFile { filePath };

    std::string_view lineView;
    size_t index;

    std::string_view subject;
    std::string_view predicate;
    std::string_view object;

    uint32_t currentIndex = 0;
    uint32_t predicateIndex = 0;

    std::unordered_map<std::string, Vertex> labelToIndex;
    std::unordered_map<std::string, Vertex> labelToLabel;

    std::vector<std::tuple<Vertex, Vertex, Label>> edges;

    // parse edge data
    while (std::getline(graphFile, line)) {
        lineView = line;

        if (lineView[0] == '\"') {
            // Find closing.
            index = lineView.find_first_of('\"', 1);
            subject = lineView.substr(0, index + 1);
            lineView = lineView.substr(index + 2);
        } else {
            index = lineView.find_first_of(' ');
            subject = lineView.substr(0, index);
            lineView = lineView.substr(index + 1);
        }

        // For now skip predicate.
        if (lineView[0] == '\"') {
            // Find closing.
            index = lineView.find_first_of('\"', 1);
            predicate = lineView.substr(0, index + 1);
            lineView = lineView.substr(index + 2);
        } else {
            index = lineView.find_first_of(' ');
            predicate = lineView.substr(0, index);
            lineView = lineView.substr(index + 1);
        }

        if (lineView[0] == '\"') {
            // Find closing.
            index = lineView.find_first_of('\"', 1);
            object = lineView.substr(0, index + 1);
        } else {
            index = lineView.find_first_of(' ');
            object = lineView.substr(0, index);
        }

        std::string subjectStr = { subject.data(), subject.size() };
        std::string predicateStr = { predicate.data(), predicate.size() };
        std::string objectStr = { object.data(), object.size() };

        if (labelToIndex.count(subjectStr) == 0) {
            labelToIndex.emplace(subjectStr, currentIndex++);
        }

        if (labelToLabel.count(predicateStr) == 0) {
            labelToLabel.emplace(predicateStr, predicateIndex++);
        }

        if (labelToIndex.count(objectStr) == 0) {
            labelToIndex.emplace(objectStr, currentIndex++);
        }

        auto source = labelToIndex[subjectStr];
        auto label = labelToLabel[predicateStr];
        auto target = labelToIndex[objectStr];

        edges.emplace_back(source, target, label);
    }

    return createLabeledGraph(edges, labelToIndex.size(), labelToLabel.size());
}

std::unique_ptr<LabeledEdgeGraph> RdfGraphReader::readLabeledEdgeGraph(const std::string &filePath) {
    std::string line;
    std::ifstream graphFile { filePath };

    std::string_view lineView;
    size_t index;

    std::string_view subject;
    std::string_view predicate;
    std::string_view object;

    uint32_t currentIndex = 0;
    uint32_t predicateIndex = 0;

    std::unordered_map<std::string, Vertex> labelToIndex;
    std::unordered_map<std::string, Vertex> labelToLabel;

    std::vector<std::tuple<Vertex, Vertex, Label>> edges;

    // parse edge data
    while (std::getline(graphFile, line)) {
        lineView = line;

        if (lineView[0] == '\"') {
            // Find closing.
            index = lineView.find_first_of('\"', 1);
            subject = lineView.substr(0, index + 1);
            lineView = lineView.substr(index + 2);
        } else {
            index = lineView.find_first_of(' ');
            subject = lineView.substr(0, index);
            lineView = lineView.substr(index + 1);
        }

        // For now skip predicate.
        if (lineView[0] == '\"') {
            // Find closing.
            index = lineView.find_first_of('\"', 1);
            predicate = lineView.substr(0, index + 1);
            lineView = lineView.substr(index + 2);
        } else {
            index = lineView.find_first_of(' ');
            predicate = lineView.substr(0, index);
            lineView = lineView.substr(index + 1);
        }

        if (lineView[0] == '\"') {
            // Find closing.
            index = lineView.find_first_of('\"', 1);
            object = lineView.substr(0, index + 1);
        } else {
            index = lineView.find_first_of(' ');
            object = lineView.substr(0, index);
        }

        std::string subjectStr = { subject.data(), subject.size() };
        std::string predicateStr = { predicate.data(), predicate.size() };
        std::string objectStr = { object.data(), object.size() };

        if (labelToIndex.count(subjectStr) == 0) {
            labelToIndex.emplace(subjectStr, currentIndex++);
        }

        if (labelToLabel.count(predicateStr) == 0) {
            labelToLabel.emplace(predicateStr, predicateIndex++);
        }

        if (labelToIndex.count(objectStr) == 0) {
            labelToIndex.emplace(objectStr, currentIndex++);
        }

        auto source = labelToIndex[subjectStr];
        auto label = labelToLabel[predicateStr];
        auto target = labelToIndex[objectStr];

        edges.emplace_back(source, target, label);
    }

    auto graph = std::unique_ptr<LabeledEdgeGraph>();
    graph->setSizes(labelToIndex.size(), labelToLabel.size(), edges.size());

    for (auto &edge : edges) {
        graph->addEdge(std::get<0>(edge), std::get<1>(edge), std::get<2>(edge));
    }

    return graph;
}

std::unique_ptr<PerLabelGraph> RdfGraphReader::readPerLabelGraph(const std::string &filePath) {
    std::string line;
    std::ifstream graphFile { filePath };

    std::string_view lineView;
    size_t index;

    std::string_view subject;
    std::string_view predicate;
    std::string_view object;

    uint32_t currentIndex = 0;
    uint32_t predicateIndex = 0;

    std::unordered_map<std::string, Vertex> labelToIndex;
    std::unordered_map<std::string, Vertex> labelToLabel;

    std::vector<std::tuple<Vertex, Vertex, Label>> edges;

    // parse edge data
    while (std::getline(graphFile, line)) {
        lineView = line;

        if (lineView[0] == '\"') {
            // Find closing.
            index = lineView.find_first_of('\"', 1);
            subject = lineView.substr(0, index + 1);
            lineView = lineView.substr(index + 2);
        } else {
            index = lineView.find_first_of(' ');
            subject = lineView.substr(0, index);
            lineView = lineView.substr(index + 1);
        }

        // For now skip predicate.
        if (lineView[0] == '\"') {
            // Find closing.
            index = lineView.find_first_of('\"', 1);
            predicate = lineView.substr(0, index + 1);
            lineView = lineView.substr(index + 2);
        } else {
            index = lineView.find_first_of(' ');
            predicate = lineView.substr(0, index);
            lineView = lineView.substr(index + 1);
        }

        if (lineView[0] == '\"') {
            // Find closing.
            index = lineView.find_first_of('\"', 1);
            object = lineView.substr(0, index + 1);
        } else {
            index = lineView.find_first_of(' ');
            object = lineView.substr(0, index);
        }

        std::string subjectStr = { subject.data(), subject.size() };
        std::string predicateStr = { predicate.data(), predicate.size() };
        std::string objectStr = { object.data(), object.size() };

        if (labelToIndex.count(subjectStr) == 0) {
            labelToIndex.emplace(subjectStr, currentIndex++);
        }

        if (labelToLabel.count(predicateStr) == 0) {
            labelToLabel.emplace(predicateStr, predicateIndex++);
        }

        if (labelToIndex.count(objectStr) == 0) {
            labelToIndex.emplace(objectStr, currentIndex++);
        }

        auto source = labelToIndex[subjectStr];
        auto label = labelToLabel[predicateStr];
        auto target = labelToIndex[objectStr];

        edges.emplace_back(source, target, label);
    }

    auto graph = std::unique_ptr<PerLabelGraph>();
    graph->setSizes(labelToIndex.size(), labelToLabel.size());

    for (auto &edge : edges) {
        graph->addEdge(std::get<0>(edge), std::get<1>(edge), std::get<2>(edge));
    }

    return graph;
}
