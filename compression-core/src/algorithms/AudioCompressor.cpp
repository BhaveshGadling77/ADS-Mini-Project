#include "algorithms/audio/AudioCompressor.hpp"
#include <stdexcept>

namespace adsc {

AudioCompressor::AudioCompressor(Algorithm algo) : algorithm(algo) {}

std::string AudioCompressor::getName() const {
    switch (algorithm) {
        case Algorithm::AUDIO_WAVELET: return "1D Wavelet Tree (Audio - Parth)";
        case Algorithm::AUDIO_DELTA_RLE: return "Delta Encoding + RLE (Audio - Parth)";
        case Algorithm::AUDIO_LPC: return "Linear Predictive Coding + Huffman (Audio - Parth)";
        default: return "Audio Compression (Parth)";
    }
}

CompressionMetrics AudioCompressor::compress(std::istream& /*in*/, std::ostream& /*out*/) {
    throw std::runtime_error(
        "Audio compression algorithm '" + getName() + "' is assigned to Parth and awaiting implementation. "
        "Please implement compress() in `src/algorithms/AudioCompressor.cpp` (see docs/development/TEAM_IMPLEMENTATION_GUIDE.md)."
    );
}

void AudioCompressor::decompress(std::istream& /*in*/, std::ostream& /*out*/, uint64_t /*originalSize*/) {
    throw std::runtime_error(
        "Audio decompression algorithm '" + getName() + "' is assigned to Parth and awaiting implementation. "
        "Please implement decompress() in `src/algorithms/AudioCompressor.cpp` (see docs/development/TEAM_IMPLEMENTATION_GUIDE.md)."
    );
}

} // namespace adsc
