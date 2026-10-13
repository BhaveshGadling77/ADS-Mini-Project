#include "algorithms/video/VideoCompressor.hpp"
#include <stdexcept>

namespace adsc {

VideoCompressor::VideoCompressor(Algorithm algo) : algorithm(algo) {}

std::string VideoCompressor::getName() const {
    switch (algorithm) {
        case Algorithm::VIDEO_INTERFRAME: return "Inter-frame Quadtree (Video - Sangram)";
        case Algorithm::VIDEO_OCTREE: return "3D Octree (Video - Sangram)";
        default: return "Video Compression (Sangram)";
    }
}

CompressionMetrics VideoCompressor::compress(std::istream& /*in*/, std::ostream& /*out*/) {
    throw std::runtime_error(
        "Video compression algorithm '" + getName() + "' is assigned to Sangram and awaiting implementation. "
        "Please implement compress() in `src/algorithms/VideoCompressor.cpp` (see docs/development/TEAM_IMPLEMENTATION_GUIDE.md)."
    );
}

void VideoCompressor::decompress(std::istream& /*in*/, std::ostream& /*out*/, uint64_t /*originalSize*/) {
    throw std::runtime_error(
        "Video decompression algorithm '" + getName() + "' is assigned to Sangram and awaiting implementation. "
        "Please implement decompress() in `src/algorithms/VideoCompressor.cpp` (see docs/development/TEAM_IMPLEMENTATION_GUIDE.md)."
    );
}

} // namespace adsc
