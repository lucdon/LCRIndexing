#include "utility/Format.hpp"

static constexpr uint64_t max_memory[] = {                  // types:
        1ull,                                               // bytes
        1ull * 1000,                                        // kilo bytes
        1ull * 1000 * 1000,                                 // mega bytes
        1ull * 1000 * 1000 * 1000,                          // giga bytes
        1ull * 1000 * 1000 * 1000 * 1000,                   // tera bytes
        1ull * 1000 * 1000 * 1000 * 1000 * 1000,            // peta bytes
        1ull * 1000 * 1000 * 1000 * 1000 * 1000 * 1000      //  exa bytes
};

static constexpr double max_time[] = {                      // types:
        1ull,                                               // nano seconds
        1ull * 1000,                                        // micro seconds
        1ull * 1000 * 1000,                                 // milli seconds
        1ull * 1000 * 1000 * 1000,                          // seconds
        1ull * 1000 * 1000 * 1000 * 60,                     // minutes
        1ull * 1000 * 1000 * 1000 * 60 * 60,                // hours
        1ull * 1000 * 1000 * 1000 * 60 * 60 * 24,           // days
        1ull * 1000 * 1000 * 1000 * 60 * 60 * 24 * 7        // weeks
};

static constexpr const char *suffixes_memory[] = { "B ", "KB", "MB", "GB", "TB", "PB" };
static constexpr uint32_t suffixes_memory_length = std::size(suffixes_memory);

static constexpr const char *suffixes_time[] = { "ns   ", "µs   ", "ms   ", "s    ", "mins ", "hours", "days " };
static constexpr uint32_t suffixes_time_length = std::size(suffixes_time);

std::ostream &formatMemory(std::ostream &out, uint64_t value) {
    for (auto i = 0u; i < suffixes_memory_length; i++) {
        if (value > max_memory[i + 1]) {
            continue;
        }

        double rounded = double(value) / double(max_memory[i]);

        formatThreeDigits(out, rounded);
        out << " " << suffixes_memory[i];
        return out;
    }

    double rounded = double(value) / double(max_memory[suffixes_memory_length]);

    formatThreeDigits(out, rounded);
    out << " EB";
    return out;
}

std::ostream &formatTime(std::ostream &out, double timeInNs) {
    for (auto i = 0u; i < suffixes_time_length; i++) {
        if (timeInNs > max_time[i + 1]) {
            continue;
        }

        double rounded = timeInNs / double(max_time[i]);

        formatThreeDigits(out, rounded);
        out << " " << suffixes_time[i];
        return out;
    }

    double rounded = timeInNs / double(max_time[suffixes_time_length]);

    formatThreeDigits(out, rounded);
    out << " weeks";
    return out;
}