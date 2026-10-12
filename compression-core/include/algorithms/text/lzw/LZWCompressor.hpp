#pragma once

#include "core/ICompressor.hpp"
#include "datastructures/trie/Trie.hpp"
#include <vector>
#include <string>

namespace adsc {

class LZWCompressor : public ICompressor {
public:
    LZWCompressor();
    ~LZWCompressor() override = default;

    Algorithm getAlgorithm() const override { return Algorithm::LZW; }
    std::string getName() const override { return "LZW (Trie-based)"; }

    CompressionMetrics compress(std::istream& in, std::ostream& out) override;
    void decompress(std::istream& in, std::ostream& out, uint64_t originalSize) override;

private:
    static constexpr int INIT_BITS = 9;
    static constexpr int MAX_BITS = 16;
    static constexpr uint32_t MAX_CODES = (1U << MAX_BITS) - 1;
};

} // namespace adsc
