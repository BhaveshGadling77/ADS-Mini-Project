#pragma once

#include "core/ICompressor.hpp"
#include <vector>
#include <string>

namespace adsc {

class BWTCompressor : public ICompressor {
public:
    BWTCompressor();
    ~BWTCompressor() override = default;

    Algorithm getAlgorithm() const override { return Algorithm::BWT_MTF; }
    std::string getName() const override { return "BWT + MTF + RLE"; }

    CompressionMetrics compress(std::istream& in, std::ostream& out) override;
    void decompress(std::istream& in, std::ostream& out, uint64_t originalSize) override;

private:
    struct BWTResult {
        std::vector<uint8_t> transformed;
        uint32_t primaryIndex = 0;
    };

    BWTResult forwardBWT(const std::vector<uint8_t>& input);
    std::vector<uint8_t> inverseBWT(const std::vector<uint8_t>& l, uint32_t primaryIndex);

    std::vector<uint8_t> forwardMTF(const std::vector<uint8_t>& input);
    std::vector<uint8_t> inverseMTF(const std::vector<uint8_t>& input);

    std::vector<uint8_t> encodeRLE(const std::vector<uint8_t>& input);
    std::vector<uint8_t> decodeRLE(const std::vector<uint8_t>& input, size_t expectedSize);
};

} // namespace adsc
