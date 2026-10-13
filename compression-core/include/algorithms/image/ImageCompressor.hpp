#pragma once

#include "core/ICompressor.hpp"
#include <string>

namespace adsc {

/**
 * @brief Base class for Image Compression algorithms.
 * Assigned to: Jibran
 *
 * Supported algorithms:
 *  1. Spatial Quadtree (Algorithm::IMAGE_QUADTREE)
 *  2. Color Quantization with KD-Tree (Algorithm::IMAGE_QUANTIZATION)
 *  3. 2D Haar Wavelets (Algorithm::IMAGE_WAVELET)
 *
 * Teammate Implementation Guide:
 * To implement your algorithm, implement the methods in `src/algorithms/ImageCompressor.cpp`:
 *   - compress(std::istream& in, std::ostream& out): Reads raw image bytes (e.g. BMP/PPM/PNG),
 *     builds the Quadtree/KD-Tree/Wavelet transform, writes header and encoded bytes to `out`.
 *   - decompress(std::istream& in, std::ostream& out, uint64_t originalSize): Reconstructs
 *     the original image bytes and writes to `out`.
 */
class ImageCompressor : public ICompressor {
public:
    explicit ImageCompressor(Algorithm algo);
    ~ImageCompressor() override = default;

    Algorithm getAlgorithm() const override { return algorithm; }
    std::string getName() const override;

    CompressionMetrics compress(std::istream& in, std::ostream& out) override;
    void decompress(std::istream& in, std::ostream& out, uint64_t originalSize) override;

private:
    Algorithm algorithm;
};

} // namespace adsc
