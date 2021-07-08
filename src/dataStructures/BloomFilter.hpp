#pragma once

#include "graphs/Definitions.hpp"

uint32_t hashFunc(uint32_t input);
uint32_t hashFunc(uint32_t input, uint32_t index);

class BloomFilter {
private:
    boost::dynamic_bitset<> bitVector;

public:
    void setup(uint32_t bits) {
        bitVector.resize(bits);
    }

    [[nodiscard]] bool empty() const {
        return bitVector.empty();
    }

    BloomFilter &operator |=(const BloomFilter &rhs) {
        bitVector |= rhs.bitVector;
        return *this;
    }

    void add(Vertex vertex) {
        auto hash = boost::hash_value(vertex);

        bitVector[hash % bitVector.size()] = true;
    }

    void add(Vertex vertex, const LabelSet &labelSet) {
        auto hash = boost::hash_value(vertex);
        boost::hash_combine(hash, boost::hash_value(labelSet));

        bitVector[hash % bitVector.size()] = true;
    }

    void add(Vertex vertex, const Vertex &label) {
        auto hash = boost::hash_value(vertex);
        boost::hash_combine(hash, boost::hash_value(label));

        bitVector[hash % bitVector.size()] = true;
    }

    void addWithSuperSets(Vertex vertex, const LabelSet &labelSet) {
        add(vertex, labelSet);

        for (uint32_t i = 0; i < labelSet.size(); i++) {
            if (labelSet[i]) {
                continue;
            }

            LabelSet next(labelSet.size());
            next |= labelSet;
            next[i] = true;

            addWithSuperSets(vertex, next, i + 1);
        }
    }

private:
    void addWithSuperSets(Vertex vertex, const LabelSet &labelSet, uint32_t i) {
        add(vertex, labelSet);

        for (; i < labelSet.size(); i++) {
            if (labelSet[i]) {
                continue;
            }

            LabelSet next(labelSet.size());
            next |= labelSet;
            next[i] = true;

            addWithSuperSets(vertex, next, i + 1);
        }
    }
public:

    [[nodiscard]] bool contains(Vertex vertex) const {
        auto hash = boost::hash_value(vertex);
        return bitVector[hash % bitVector.size()];
    }

    [[nodiscard]] bool containsHash(size_t hash) const {
        return bitVector[hash % bitVector.size()];
    }

    [[nodiscard]] bool containsAt(size_t pos) const {
        return bitVector[pos];
    }

    [[nodiscard]] bool contains(Vertex vertex, const LabelSet &labelSet) const {
        auto hash = boost::hash_value(vertex);
        boost::hash_combine(hash, boost::hash_value(labelSet));
        return bitVector[hash % bitVector.size()];
    }

    [[nodiscard]] bool contains(Vertex vertex, Vertex label) const {
        auto hash = boost::hash_value(vertex);
        boost::hash_combine(hash, boost::hash_value(label));
        return bitVector[hash % bitVector.size()];
    }

    [[nodiscard]] size_t sizeInBytes() const {
        return bitVector.capacity() / 8;
    }
};

class BloomFilter1Hash {
private:
    boost::dynamic_bitset<> bitVector;

public:
    void setup(uint32_t bits) {
        bitVector.resize(bits);
    }

    [[nodiscard]] bool empty() const {
        return bitVector.empty();
    }

    BloomFilter1Hash &operator |=(const BloomFilter1Hash &rhs) {
        bitVector |= rhs.bitVector;
        return *this;
    }

    void add(Vertex vertex) {
        auto hash = hashFunc(vertex);
        bitVector[hash % bitVector.size()] = true;
    }

    [[nodiscard]] bool contains(Vertex vertex) const {
        auto hash = hashFunc(vertex);
        return bitVector[hash % bitVector.size()];
    }

    [[nodiscard]] size_t sizeInBytes() const {
        return bitVector.capacity() / 8;
    }
};

class BloomFilter2Hash {
private:
    boost::dynamic_bitset<> bitVector;

public:
    void setup(uint32_t bits) {
        bitVector.resize(bits);
    }

    [[nodiscard]] bool empty() const {
        return bitVector.empty();
    }

    BloomFilter2Hash &operator |=(const BloomFilter2Hash &rhs) {
        bitVector |= rhs.bitVector;
        return *this;
    }

    void add(Vertex vertex) {
        auto hash = hashFunc(vertex, 0);
        bitVector[hash % bitVector.size()] = true;

        hash = hashFunc(vertex, 1);
        bitVector[hash % bitVector.size()] = true;
    }

    [[nodiscard]] bool contains(Vertex vertex) const {
        auto hash = hashFunc(vertex, 0);

        if(!bitVector[hash % bitVector.size()]) {
            return false;
        }

        hash = hashFunc(vertex, 1);
        return bitVector[hash % bitVector.size()];
    }

    [[nodiscard]] size_t sizeInBytes() const {
        return bitVector.capacity() / 8;
    }
};

class BloomFilter3Hash {
private:
    boost::dynamic_bitset<> bitVector;

public:
    void setup(uint32_t bits) {
        bitVector.resize(bits);
    }

    [[nodiscard]] bool empty() const {
        return bitVector.empty();
    }

    BloomFilter3Hash &operator |=(const BloomFilter3Hash &rhs) {
        bitVector |= rhs.bitVector;
        return *this;
    }

    void add(Vertex vertex) {
        auto hash = hashFunc(vertex, 0);
        bitVector[(hash >> 5) % bitVector.size()] = true;

        hash = hashFunc(vertex, 1);
        bitVector[(hash >> 5) % bitVector.size()] = true;

        hash = hashFunc(vertex, 2);
        bitVector[(hash >> 5) % bitVector.size()] = true;
    }

    [[nodiscard]] bool contains(Vertex vertex) const {
        auto hash = hashFunc(vertex, 0);

        if(!bitVector[(hash >> 5) % bitVector.size()]) {
            return false;
        }

        hash = hashFunc(vertex, 1);

        if(!bitVector[(hash >> 5) % bitVector.size()]) {
            return false;
        }

        hash = hashFunc(vertex, 2);
        return bitVector[(hash >> 5) % bitVector.size()];
    }

    [[nodiscard]] size_t sizeInBytes() const {
        return bitVector.capacity() / 8;
    }
};

class BloomFilterDynamic {
private:
    boost::dynamic_bitset<> bitVector;
    uint32_t hashFunctions = 1;

public:
    void setup(uint32_t expectedElements, double probability) {
        uint32_t bits = std::ceil(
                (double(expectedElements) * std::log(probability) / std::log(1 / std::pow(2, std::log(2)))));
        hashFunctions = (uint32_t) std::round((double(bits) / double(expectedElements)) * std::log(2));
        bitVector.resize(bits);
    }

    [[nodiscard]] bool empty() const {
        return bitVector.empty();
    }

    void add(Vertex vertex);
    [[nodiscard]] bool contains(Vertex vertex) const;

    [[nodiscard]] size_t sizeInBytes() const {
        return bitVector.capacity() / 8;
    }
};