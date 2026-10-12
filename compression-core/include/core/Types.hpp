#pragma once

#include <cstdint>
#include <string>
#include <chrono>
#include <stdexcept>

namespace adsc {

enum class Modality {
    TEXT = 1,
    IMAGE = 2,
    AUDIO = 3,
    VIDEO = 4
};

inline std::string modalityToString(Modality mod) {
    switch (mod) {
        case Modality::TEXT: return "text";
        case Modality::IMAGE: return "image";
        case Modality::AUDIO: return "audio";
        case Modality::VIDEO: return "video";
        default: return "unknown";
    }
}

inline Modality stringToModality(const std::string& str) {
    if (str == "text" || str == "TEXT") return Modality::TEXT;
    if (str == "image" || str == "IMAGE") return Modality::IMAGE;
    if (str == "audio" || str == "AUDIO") return Modality::AUDIO;
    if (str == "video" || str == "VIDEO") return Modality::VIDEO;
    return Modality::TEXT;
}

enum class Algorithm {
    // Text (Bhavesh)
    LZW = 1,
    HUFFMAN = 2,
    BWT_MTF = 3,

    // Image (Jibran)
    IMAGE_QUADTREE = 4,
    IMAGE_QUANTIZATION = 5,
    IMAGE_WAVELET = 6,

    // Audio (Parth)
    AUDIO_WAVELET = 7,
    AUDIO_DELTA_RLE = 8,
    AUDIO_LPC = 9,

    // Video (Sangram)
    VIDEO_INTERFRAME = 10,
    VIDEO_OCTREE = 11
};

inline std::string algorithmToString(Algorithm algo) {
    switch (algo) {
        case Algorithm::LZW: return "lzw";
        case Algorithm::HUFFMAN: return "huffman";
        case Algorithm::BWT_MTF: return "bwt";
        case Algorithm::IMAGE_QUADTREE: return "image_quadtree";
        case Algorithm::IMAGE_QUANTIZATION: return "image_quantization";
        case Algorithm::IMAGE_WAVELET: return "image_wavelet";
        case Algorithm::AUDIO_WAVELET: return "audio_wavelet";
        case Algorithm::AUDIO_DELTA_RLE: return "audio_delta_rle";
        case Algorithm::AUDIO_LPC: return "audio_lpc";
        case Algorithm::VIDEO_INTERFRAME: return "video_interframe";
        case Algorithm::VIDEO_OCTREE: return "video_octree";
        default: return "unknown";
    }
}

inline Algorithm stringToAlgorithm(const std::string& str) {
    if (str == "lzw" || str == "LZW") return Algorithm::LZW;
    if (str == "huffman" || str == "HUFFMAN") return Algorithm::HUFFMAN;
    if (str == "bwt" || str == "BWT" || str == "bwt_mtf" || str == "BWT_MTF") return Algorithm::BWT_MTF;

    // Image
    if (str == "image_quadtree" || str == "quadtree") return Algorithm::IMAGE_QUADTREE;
    if (str == "image_quantization" || str == "quantization" || str == "kdtree") return Algorithm::IMAGE_QUANTIZATION;
    if (str == "image_wavelet" || str == "haar") return Algorithm::IMAGE_WAVELET;

    // Audio
    if (str == "audio_wavelet" || str == "wavelet_tree") return Algorithm::AUDIO_WAVELET;
    if (str == "audio_delta_rle" || str == "delta_rle") return Algorithm::AUDIO_DELTA_RLE;
    if (str == "audio_lpc" || str == "lpc") return Algorithm::AUDIO_LPC;

    // Video
    if (str == "video_interframe" || str == "interframe") return Algorithm::VIDEO_INTERFRAME;
    if (str == "video_octree" || str == "octree") return Algorithm::VIDEO_OCTREE;

    throw std::invalid_argument("Unknown algorithm: " + str);
}

struct CompressionMetrics {
    std::string algorithm;
    std::string modality = "text";
    uint64_t originalSizeBytes = 0;
    uint64_t compressedSizeBytes = 0;
    double compressionRatio = 1.0;
    double spaceSavedPercent = 0.0;
    double executionTimeMs = 0.0;
    bool success = false;
    std::string errorMessage;
};

} // namespace adsc
