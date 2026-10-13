#include "core/Engine.hpp"
#include "algorithms/text/lzw/LZWCompressor.hpp"
#include "algorithms/text/huffman/HuffmanCompressor.hpp"
#include "algorithms/text/bwt/BWTCompressor.hpp"
#include "algorithms/image/ImageCompressor.hpp"
#include "algorithms/audio/AudioCompressor.hpp"
#include "algorithms/video/VideoCompressor.hpp"
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <chrono>

namespace adsc {

std::unique_ptr<ICompressor> CompressionEngine::createCompressor(Algorithm algo) {
    switch (algo) {
        case Algorithm::LZW:
            return std::make_unique<LZWCompressor>();
        case Algorithm::HUFFMAN:
            return std::make_unique<HuffmanCompressor>();
        case Algorithm::BWT_MTF:
            return std::make_unique<BWTCompressor>();
        case Algorithm::IMAGE_QUADTREE:
        case Algorithm::IMAGE_QUANTIZATION:
        case Algorithm::IMAGE_WAVELET:
            return std::make_unique<ImageCompressor>(algo);
        case Algorithm::AUDIO_WAVELET:
        case Algorithm::AUDIO_DELTA_RLE:
        case Algorithm::AUDIO_LPC:
            return std::make_unique<AudioCompressor>(algo);
        case Algorithm::VIDEO_INTERFRAME:
        case Algorithm::VIDEO_OCTREE:
            return std::make_unique<VideoCompressor>(algo);
        default:
            throw std::invalid_argument("Unsupported algorithm");
    }
}

CompressionMetrics CompressionEngine::compressFile(
    const std::string& inputPath,
    const std::string& outputPath,
    Algorithm algo) {

    std::ifstream inFile(inputPath, std::ios::binary);
    if (!inFile.is_open()) {
        throw std::runtime_error("Cannot open input file: " + inputPath);
    }

    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile.is_open()) {
        throw std::runtime_error("Cannot open output file: " + outputPath);
    }

    auto compressor = createCompressor(algo);
    CompressionMetrics metrics = compressor->compress(inFile, outFile);
    outFile.close();
    inFile.close();

    // Verify compressed file size on disk
    if (std::filesystem::exists(outputPath)) {
        metrics.compressedSizeBytes = std::filesystem::file_size(outputPath);
        if (metrics.originalSizeBytes > 0 && metrics.compressedSizeBytes > 0) {
            metrics.compressionRatio = static_cast<double>(metrics.originalSizeBytes) / static_cast<double>(metrics.compressedSizeBytes);
            metrics.spaceSavedPercent = (1.0 - static_cast<double>(metrics.compressedSizeBytes) / static_cast<double>(metrics.originalSizeBytes)) * 100.0;
        }
    }

    return metrics;
}

CompressionMetrics CompressionEngine::decompressFile(
    const std::string& inputPath,
    const std::string& outputPath,
    std::optional<Algorithm> algoOverride) {

    auto startTime = std::chrono::high_resolution_clock::now();

    std::ifstream inFile(inputPath, std::ios::binary);
    if (!inFile.is_open()) {
        throw std::runtime_error("Cannot open input file: " + inputPath);
    }

    Header header = Header::read(inFile);
    Algorithm algo = algoOverride.value_or(static_cast<Algorithm>(header.algorithm));

    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile.is_open()) {
        throw std::runtime_error("Cannot open output file: " + outputPath);
    }

    auto compressor = createCompressor(algo);
    compressor->decompress(inFile, outFile, header.originalSize);

    outFile.close();
    inFile.close();

    auto endTime = std::chrono::high_resolution_clock::now();
    double durationMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    uint64_t restoredSize = std::filesystem::exists(outputPath) ? std::filesystem::file_size(outputPath) : 0;
    uint64_t compressedSize = std::filesystem::exists(inputPath) ? std::filesystem::file_size(inputPath) : 0;

    CompressionMetrics metrics;
    metrics.algorithm = algorithmToString(algo);
    metrics.originalSizeBytes = restoredSize;
    metrics.compressedSizeBytes = compressedSize;
    metrics.compressionRatio = (compressedSize > 0) ? (static_cast<double>(restoredSize) / static_cast<double>(compressedSize)) : 1.0;
    metrics.spaceSavedPercent = 0.0;
    metrics.executionTimeMs = durationMs;
    metrics.success = true;

    return metrics;
}

FileInfo CompressionEngine::inspectFile(const std::string& inputPath) {
    FileInfo info;
    if (!std::filesystem::exists(inputPath)) {
        return info;
    }

    info.fileSize = std::filesystem::file_size(inputPath);

    std::ifstream inFile(inputPath, std::ios::binary);
    if (!inFile.is_open()) {
        return info;
    }

    try {
        Header hdr = Header::read(inFile);
        info.isCompressed = true;
        info.version = hdr.version;
        info.originalSize = hdr.originalSize;
        info.algorithm = algorithmToString(static_cast<Algorithm>(hdr.algorithm));
    } catch (...) {
        info.isCompressed = false;
    }

    return info;
}

} // namespace adsc
