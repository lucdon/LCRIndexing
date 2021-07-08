#pragma once

#include "utility/Format.hpp"

class Limit {
public:
    Limit() = default;
    virtual ~Limit() = default;

    Limit(const Limit &) = default;
    Limit(Limit &&) = default;

    Limit &operator =(const Limit &) = default;
    Limit &operator =(Limit &&) = default;

    virtual void begin() {

    }

    virtual bool checkLimit() = 0;
    virtual void print() = 0;
};

class LimitRunner {
private:
    std::unique_ptr<std::thread> threadPtr = nullptr;
    Limit *limit;

    bool cancelled = false;
public:
    explicit LimitRunner(std::unique_ptr<Limit> &lim) : limit(lim.get()) { }

    void start() {
        if (limit == nullptr) {
            return;
        }

        std::cout << "running with limits:" << std::endl;
        limit->print();
        std::cout << std::endl;

        limit->begin();
        threadPtr = std::make_unique<std::thread>(limitRun, this);
    }

    void stop() {
        // If there was no limit then nothing to do.
        if (limit == nullptr) {
            return;
        }

        // Otherwise cancel the limit thread and finish.
        cancelled = true;
        threadPtr->join();
    }

private:
    static void limitRun(LimitRunner *limitRunner) {
        while (!limitRunner->cancelled) {
            if (limitRunner->limit->checkLimit()) {
                std::cerr << std::fatal;
                return;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
};

class TimeLimit : public Limit {
private:
    uint64_t maxTimeInMillis;
    std::chrono::steady_clock::time_point start;
public:
    template<typename TDur>
    explicit TimeLimit(TDur maxTimeDur) {
        maxTimeInMillis = uint64_t(std::chrono::duration_cast<std::chrono::milliseconds>(maxTimeDur).count());
    }

    explicit TimeLimit(uint32_t maxTimeInSeconds) {
        maxTimeInMillis = uint64_t(maxTimeInSeconds) * 1000ull;
    }

    void begin() override {
        start = std::chrono::steady_clock::now();
    }

    bool checkLimit() override {
        auto end = std::chrono::steady_clock::now();
        auto duration = uint64_t(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());

        if (duration > maxTimeInMillis) {
            std::cerr << "time limit of ";
            formatTime(std::cerr, maxTimeInMillis * 1000ull * 1000ull);
            std::cerr << " reached, terminating ...";
            return true;
        }

        return false;
    }

    void print() override {
        std::cout << "time limit of ";
        formatTime(std::cout, maxTimeInMillis * 1000ull * 1000ull);
        std::cout << std::endl;
    }
};

class MemoryLimit : public Limit {
    uint64_t maxMemoryInBytes;
public:
    explicit MemoryLimit(uint32_t megaBytes) {
        maxMemoryInBytes = uint64_t(megaBytes) * 1000ull * 1000ull;
    }

    bool checkLimit() override {
        if (getCurrentPSS() > maxMemoryInBytes) {
            std::cerr << "memory limit of ";
            formatMemory(std::cerr, maxMemoryInBytes);
            std::cerr << " reached, terminating ...";
            return true;
        }

        return false;
    }

    void print() override {
        std::cout << "memory limit of ";
        formatMemory(std::cout, maxMemoryInBytes);
        std::cout << std::endl;
    }
};

class MultiLimit : public Limit {
private:
    std::vector<std::shared_ptr<Limit>> limits;

public:
    void addLimit(const std::shared_ptr<Limit> &limit) {
        limits.emplace_back(limit);
    }

    void addLimit(std::shared_ptr<Limit> &&limit) {
        limits.emplace_back(limit);
    }

    void begin() override {
        for (auto &limit : limits) {
            limit->begin();
        }
    }

    bool checkLimit() override {
        for (auto &limit : limits) {
            if (limit->checkLimit()) {
                return true;
            }
        }

        return false;
    }

    void print() override {
        for (auto &limit : limits) {
            limit->print();
        }
    }
};