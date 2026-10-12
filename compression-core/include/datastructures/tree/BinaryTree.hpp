#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace adsc {

struct HuffmanNode {
    uint8_t byteValue = 0;
    uint64_t frequency = 0;
    std::shared_ptr<HuffmanNode> left = nullptr;
    std::shared_ptr<HuffmanNode> right = nullptr;

    HuffmanNode(uint8_t byte, uint64_t freq)
        : byteValue(byte), frequency(freq), left(nullptr), right(nullptr) {}

    HuffmanNode(uint64_t freq, std::shared_ptr<HuffmanNode> l, std::shared_ptr<HuffmanNode> r)
        : byteValue(0), frequency(freq), left(std::move(l)), right(std::move(r)) {}

    bool isLeaf() const {
        return !left && !right;
    }
};

struct HuffmanNodeCompare {
    bool operator()(const std::shared_ptr<HuffmanNode>& a, const std::shared_ptr<HuffmanNode>& b) const {
        if (a->frequency != b->frequency) {
            return a->frequency < b->frequency; // For MinHeap, smaller frequency comes first
        }
        return a->byteValue < b->byteValue;
    }
};

} // namespace adsc
