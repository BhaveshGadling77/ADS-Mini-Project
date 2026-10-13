#pragma once

#include "core/ICompressor.hpp"
#include "datastructures/heap/MinHeap.hpp"
#include "datastructures/tree/BinaryTree.hpp"
#include <vector>
#include <string>
#include <unordered_map>

namespace adsc {

class HuffmanCompressor : public ICompressor {
public:
    HuffmanCompressor();
    ~HuffmanCompressor() override = default;

    Algorithm getAlgorithm() const override { return Algorithm::HUFFMAN; }
    std::string getName() const override { return "Static Huffman (Min-Heap + Tree)"; }

    CompressionMetrics compress(std::istream& in, std::ostream& out) override;
    void decompress(std::istream& in, std::ostream& out, uint64_t originalSize) override;

private:
    struct Code {
        std::vector<bool> bits;
    };

    void generateCodes(const std::shared_ptr<HuffmanNode>& node, std::vector<bool>& currentBits, std::vector<Code>& table);
    std::shared_ptr<HuffmanNode> buildTree(const std::vector<uint64_t>& frequencies);
};

} // namespace adsc
