#pragma once

#include "core/ICompressor.hpp"
#include <string>

namespace adsc {

/**
 * @brief Base class for Audio Compression algorithms.
 * Assigned to: Parth
 *
 * Supported algorithms:
 *  1. 1D Wavelet Tree (Algorithm::AUDIO_WAVELET)
 *  2. Delta Encoding + RLE (Algorithm::AUDIO_DELTA_RLE)
 *  3. Linear Predictive Coding + Huffman (Algorithm::AUDIO_LPC)
 *
 * Teammate Implementation Guide:
 * To implement your algorithm, implement the methods in `src/algorithms/AudioCompressor.cpp`:
 *   - compress(std::istream& in, std::ostream& out): Reads audio sample stream (e.g. WAV/PCM),
 *     transforms samples via Wavelet/Delta/LPC, writes header and encoded stream to `out`.
 *   - decompress(std::istream& in, std::ostream& out, uint64_t originalSize): Inverts the
 *     transform and restores the original audio samples to `out`.
 */
class AudioCompressor : public ICompressor {
public:
    explicit AudioCompressor(Algorithm algo);
    ~AudioCompressor() override = default;

    Algorithm getAlgorithm() const override { return algorithm; }
    std::string getName() const override;

    CompressionMetrics compress(std::istream& in, std::ostream& out) override;
    void decompress(std::istream& in, std::ostream& out, uint64_t originalSize) override;

private:
    Algorithm algorithm;
};

} // namespace adsc
