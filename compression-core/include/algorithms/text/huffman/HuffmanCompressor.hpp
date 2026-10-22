#pragma once

#include "core/ICompressor.hpp"
#include "algorithms/huffman/HuffmanCodec.hpp"

namespace adsc {

class HuffmanCompressor : public ICompressor {
public:
    HuffmanCompressor();
    ~HuffmanCompressor() override = default;

    Algorithm getAlgorithm() const override { return Algorithm::HUFFMAN; }
    std::string getName() const override { return "Static Huffman (Min-Heap + Tree)"; }

    CompressionMetrics compress(std::istream& in, std::ostream& out) override;
    void decompress(std::istream& in, std::ostream& out, uint64_t originalSize) override;

};

} // namespace adsc
