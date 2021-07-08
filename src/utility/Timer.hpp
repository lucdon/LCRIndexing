#pragma once

#include "utility/Format.hpp"

class Timer {
    const char *curTimerName = nullptr;
    std::chrono::steady_clock::time_point start;

public:
    void begin() {
        this->curTimerName = nullptr;
        start = std::chrono::steady_clock::now();
    }

    void begin(const char *timerName) {
        this->curTimerName = timerName;
        start = std::chrono::steady_clock::now();
    }

    void begin(const std::string &timerName) {
        this->curTimerName = timerName.c_str();
        start = std::chrono::steady_clock::now();
    }

    void endSameLine() {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<double, std::micro>(end - start);

        if (curTimerName != nullptr) {
            formatWidth(std::cout, curTimerName, 50);
        }

        std::cout << "Took: ";
        formatTime(std::cout, duration);
    }

    void end() {
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<double, std::micro>(end - start);

        if (curTimerName != nullptr) {
            formatWidth(std::cout, curTimerName, 50);
        }

        std::cout << "Took: ";
        formatTime(std::cout, duration);
        std::cout << std::endl;
    }
};