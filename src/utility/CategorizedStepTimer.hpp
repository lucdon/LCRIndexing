#pragma once

#include <utility>

#include "utility/Format.hpp"

class NamedStatsContainer {
private:
    double minValue = std::numeric_limits<double>::max();
    double maxValue = std::numeric_limits<double>::min();

    double totalValue = 0;
    double totalSquared = 0;

    uint32_t totalCount = 0;

    std::string categoryName;

public:
    explicit NamedStatsContainer(std::string name) : categoryName(std::move(name)) { }

    void reset() {
        totalCount = 0;

        totalValue = 0;
        totalSquared = 0;

        minValue = std::numeric_limits<double>::max();
        maxValue = std::numeric_limits<double>::min();
    }

    void addEntry(double value) {
        totalValue += value;
        minValue = std::min(minValue, value);
        maxValue = std::max(maxValue, value);
        totalSquared += value * value;
        totalCount++;
    }

    [[nodiscard]] const std::string &name() const {
        return categoryName;
    }

    [[nodiscard]] uint32_t stepCount() const {
        return totalCount;
    }

    [[nodiscard]] double total() const {
        return totalValue;
    }

    [[nodiscard]] double max() const {
        return maxValue;
    }

    [[nodiscard]] double min() const {
        return minValue;
    }

    [[nodiscard]] double avg() const {
        return totalValue / totalCount;
    }

    [[nodiscard]] double var() const {
        return (totalSquared - (totalValue * totalValue) / totalCount) / (totalCount - 1);
    }

    [[nodiscard]] double std() const {
        return std::sqrt(var());
    }
};

class CategorizedStepTimer {
    std::chrono::steady_clock::time_point start;

    std::map <uint32_t, NamedStatsContainer> categories { };
    uint32_t indent = 0;

public:
    void reset() {
        for (auto &category : categories) {
            category.second.reset();
        }
    }

    void setIndent(uint32_t indentation) {
        indent = indentation;
    }

    [[nodiscard]] bool hasCategory(uint32_t categoryId) {
        return categories.count(categoryId) != 0;
    }

    void addCategory(uint32_t categoryId, const std::string &categoryName) {
        categories.try_emplace(categoryId, categoryName);
    }

    void beginStep() {
        start = std::chrono::steady_clock::now();
    }

    template<class ... T>
    void endStep(const T &... args) {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<double, std::nano>(end - start).count();

        setStats(duration, args...);
    }
private:
    template<class ... T>
    void setStats(double duration, uint32_t categoryId, const T &... args) {
        if (categories.count(categoryId) != 0) {
            auto &category = categories.at(categoryId);
            category.addEntry(duration);
        }

        setStats(duration, args...);
    }

    void setStats(double duration) {
    }

    friend std::ostream &operator <<(std::ostream &out, const CategorizedStepTimer &timer);
};

std::ostream &operator <<(std::ostream &out, const NamedStatsContainer &container);
std::ostream &operator <<(std::ostream &out, const CategorizedStepTimer &timer);