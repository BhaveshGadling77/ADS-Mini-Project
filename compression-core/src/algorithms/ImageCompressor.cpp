#include "algorithms/image/ImageCompressor.hpp"
#include <stdexcept>

namespace adsc {

ImageCompressor::ImageCompressor(Algorithm algo) : algorithm(algo) {}

std::string ImageCompressor::getName() const {
    switch (algorithm) {
        case Algorithm::IMAGE_QUADTREE: return "Spatial Quadtree (Image - Jibran)";
        case Algorithm::IMAGE_QUANTIZATION: return "Color Quantization KD-Tree (Image - Jibran)";
        case Algorithm::IMAGE_WAVELET: return "2D Haar Wavelets (Image - Jibran)";
        default: return "Image Compression (Jibran)";
    }
}

CompressionMetrics ImageCompressor::compress(std::istream& /*in*/, std::ostream& /*out*/) {
    throw std::runtime_error(
        "Image compression algorithm '" + getName() + "' is assigned to Jibran and awaiting implementation. "
        "Please implement compress() in `src/algorithms/ImageCompressor.cpp` (see docs/development/TEAM_IMPLEMENTATION_GUIDE.md)."
    );
}

void ImageCompressor::decompress(std::istream& /*in*/, std::ostream& /*out*/, uint64_t /*originalSize*/) {
    throw std::runtime_error(
        "Image decompression algorithm '" + getName() + "' is assigned to Jibran and awaiting implementation. "
        "Please implement decompress() in `src/algorithms/ImageCompressor.cpp` (see docs/development/TEAM_IMPLEMENTATION_GUIDE.md)."
    );
}

} // namespace adsc
