#include "CSVQueryReader.hpp"

std::unique_ptr<ReachQuerySet> CSVQueryReader::readQueries(const std::string &filePath) {
    std::string line;
    std::ifstream queriesFile { filePath };

    std::string_view lineView;
    size_t firstPart;
    size_t lastPart;

    auto querySet = std::make_unique<ReachQuerySet>();

    while (std::getline(queriesFile, line)) {
        lineView = line;

        firstPart = lineView.find_first_of(',');
        lastPart = lineView.find_last_of(',');

        Vertex source;
        Vertex target;

        source = std::stoul(lineView.data());
        target = std::stoul(lineView.data() + lastPart + 1);

        querySet->emplace_back(source, target);
    }

    return querySet;
}

std::unique_ptr<LCRQuerySet> CSVQueryReader::readLabeledQueries(const std::string &filePath) {
    std::string line;
    std::ifstream queriesFile { filePath };

    std::string_view lineView;
    size_t firstPart;
    size_t lastPart;

    auto querySet = std::make_unique<LCRQuerySet>();

    while (std::getline(queriesFile, line)) {
        lineView = line;

        firstPart = lineView.find_first_of(',');
        lastPart = lineView.find_last_of(',');

        Vertex source;
        Label label;
        std::vector<Label> labels;
        Vertex target;


        source = std::stoul(lineView.data());

        auto labelPart = lineView.substr(firstPart + 1, lastPart - firstPart - 1);

        if (labelPart.back() == '+') {
            labelPart = labelPart.substr(0, labelPart.size() - 1);
        }

        auto lastPos = labelPart.find('+');

        while (lastPos != std::string_view::npos) {
            label = std::stoul(labelPart.data());
            labels.emplace_back(label);

            labelPart = labelPart.substr(lastPos + 1);
            lastPos = labelPart.find('+');
        }

        label = std::stoul(labelPart.data());
        labels.emplace_back(label);

        target = std::stoul(lineView.data() + lastPart + 1);

        querySet->emplace_back(source, target, labels);
    }

    return querySet;
}
