#pragma once

#include "core/ICompressor.hpp"
#include <string>

namespace adsc {

/**
 * @brief Base class for Video Compression algorithms.
 * Assigned to: Sangram
 *
 * Supported algorithms:
 *  1. Inter-frame Quadtree (Algorithm::VIDEO_INTERFRAME)
 *  2. 3D Octree (Algorithm::VIDEO_OCTREE)
 *
 * Teammate Implementation Guide:
 * To implement your algorithm, implement the methods in `src/algorithms/VideoCompressor.cpp`:
 *   - compress(std::istream& in, std::ostream& out): Reads video frame stream (e.g. YUV/raw frames),
 *     builds the Inter-frame Quadtree / 3D Octree, writes header and encoded delta blocks to `out`.
 *   - decompress(std::istream& in, std::ostream& out, uint64_t originalSize): Reconstructs the
 *     frame sequence from delta blocks and writes original stream to `out`.
 */
class VideoCompressor : public ICompressor {
public:
    explicit VideoCompressor(Algorithm algo);
    ~VideoCompressor() override = default;

    Algorithm getAlgorithm() const override { return algorithm; }
    std::string getName() const override;

    CompressionMetrics compress(std::istream& in, std::ostream& out) override;
    void decompress(std::istream& in, std::ostream& out, uint64_t originalSize) override;

private:
    Algorithm algorithm;
};

} // namespace adsc
