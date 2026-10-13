#pragma once

#include "core/Types.hpp"
#include "core/ICompressor.hpp"
#include "core/FileHeader.hpp"
#include <memory>
#include <string>
#include <optional>

namespace adsc {

struct FileInfo {
    bool isCompressed = false;
    std::string algorithm;
    uint8_t version = 0;
    uint64_t originalSize = 0;
    uint64_t fileSize = 0;
};

class CompressionEngine {
public:
    CompressionEngine() = default;

    static std::unique_ptr<ICompressor> createCompressor(Algorithm algo);

    CompressionMetrics compressFile(
        const std::string& inputPath,
        const std::string& outputPath,
        Algorithm algo
    );

    CompressionMetrics decompressFile(
        const std::string& inputPath,
        const std::string& outputPath,
        std::optional<Algorithm> algoOverride = std::nullopt
    );

    FileInfo inspectFile(const std::string& inputPath);
};

} // namespace adsc
