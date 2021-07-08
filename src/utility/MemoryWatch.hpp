#pragma once

#include "utility/Format.hpp"

class MemoryWatch {
    const char *curRegionName = nullptr;
    uint64_t start;

public:
    void begin() {
        this->curRegionName = nullptr;
        start = getCurrentPSS();
    }

    void begin(const char *regionName) {
        this->curRegionName = regionName;
        start = getCurrentPSS();
    }

    void begin(const std::string &regionName) {
        this->curRegionName = regionName.c_str();
        start = getCurrentPSS();
    }

    void endSameLine() {
        auto end = getCurrentPSS();
        auto diff = end - start;

        if(curRegionName != nullptr) {
            formatWidth(std::cout, curRegionName, 50);
        }

        std::cout << "Memory: ";
        formatMemory(std::cout, diff);
    }

    void end() {
        auto end = getCurrentPSS();
        auto diff = end - start;

        if(curRegionName != nullptr) {
            formatWidth(std::cout, curRegionName, 50);
        }

        std::cout << "Memory: ";
        formatMemory(std::cout, diff);
        std::cout << std::endl;
    }
};