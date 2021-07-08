#pragma once

#include "utility/Format.hpp"

class StepTimer {
    std::chrono::steady_clock::time_point start;

    double minTime = std::numeric_limits<double>::max();
    double maxTime = std::numeric_limits<double>::min();

    double totalTime = 0;
    double totalTimeSq = 0;

    uint32_t totalCount = 0;

public:
    void reset() {
        totalCount = 0;

        totalTime = 0;
        totalTimeSq = 0;

        minTime = std::numeric_limits<double>::max();
        maxTime = std::numeric_limits<double>::min();
    }

    void beginStep() {
        start = std::chrono::steady_clock::now();
    }

    void endStep() {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<double, std::nano>(end - start).count();
        totalTime += duration;
        minTime = std::min(minTime, duration);
        maxTime = std::max(maxTime, duration);
        totalTimeSq += duration * duration;
        totalCount++;
    }

    [[nodiscard]] uint32_t stepCount() const {
        return totalCount;
    }

    [[nodiscard]] double totalTimeNs() const {
        return totalTime;
    }

    [[nodiscard]] double maxTimeNs() const {
        return maxTime;
    }

    [[nodiscard]] double minTimeNs() const {
        return minTime;
    }

    [[nodiscard]] double avgTimeNs() const {
        return totalTime / totalCount;
    }

    [[nodiscard]] double varTimeNs() const {
        return (totalTimeSq - (totalTime * totalTime) / totalCount) / (totalCount - 1);
    }

    [[nodiscard]] double stdTimeNs() const {
        return std::sqrt(varTimeNs());
    }
};

std::ostream &operator <<(std::ostream &out, const StepTimer &timer);