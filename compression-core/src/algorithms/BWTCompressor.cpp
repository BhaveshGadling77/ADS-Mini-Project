#include "algorithms/text/bwt/BWTCompressor.hpp"
#include "core/FileHeader.hpp"
#include <algorithm>
#include <chrono>
#include <numeric>
#include <list>

namespace adsc {

BWTCompressor::BWTCompressor() = default;

BWTCompressor::BWTResult BWTCompressor::forwardBWT(const std::vector<uint8_t>& input) {
    size_t n = input.size();
    if (n == 0) {
        return {{}, 0};
    }

    std::vector<uint32_t> indices(n);
    std::iota(indices.begin(), indices.end(), 0);

    std::stable_sort(indices.begin(), indices.end(), [&input, n](uint32_t a, uint32_t b) {
        if (a == b) return false;
        for (size_t k = 0; k < n; ++k) {
            uint8_t ca = input[(a + k) % n];
            uint8_t cb = input[(b + k) % n];
            if (ca != cb) {
                return ca < cb;
            }
        }
        return a < b;
    });

    BWTResult result;
    result.transformed.resize(n);
    result.primaryIndex = 0;

    for (size_t i = 0; i < n; ++i) {
        if (indices[i] == 0) {
            result.primaryIndex = static_cast<uint32_t>(i);
        }
        result.transformed[i] = input[(indices[i] + n - 1) % n];
    }

    return result;
}

std::vector<uint8_t> BWTCompressor::inverseBWT(const std::vector<uint8_t>& l, uint32_t primaryIndex) {
    size_t n = l.size();
    if (n == 0) {
        return {};
    }

    std::vector<uint32_t> count(256, 0);
    for (uint8_t b : l) {
        count[b]++;
    }

    std::vector<uint32_t> c(256, 0);
    for (size_t i = 1; i < 256; ++i) {
        c[i] = c[i - 1] + count[i - 1];
    }

    std::vector<uint32_t> countBefore(256, 0);
    std::vector<uint32_t> lf(n);
    for (size_t i = 0; i < n; ++i) {
        lf[i] = c[l[i]] + countBefore[l[i]]++;
    }

    std::vector<uint8_t> output(n);
    uint32_t curr = primaryIndex;
    for (size_t step = 0; step < n; ++step) {
        if (curr >= n) {
            throw std::runtime_error("Corrupted BWT data: index out of bounds");
        }
        output[n - 1 - step] = l[curr];
        curr = lf[curr];
    }

    return output;
}

std::vector<uint8_t> BWTCompressor::forwardMTF(const std::vector<uint8_t>& input) {
    std::vector<uint8_t> list(256);
    std::iota(list.begin(), list.end(), 0);

    std::vector<uint8_t> output;
    output.reserve(input.size());

    for (uint8_t b : input) {
        auto it = std::find(list.begin(), list.end(), b);
        uint8_t pos = static_cast<uint8_t>(std::distance(list.begin(), it));
        output.push_back(pos);

        // Move to front
        list.erase(it);
        list.insert(list.begin(), b);
    }

    return output;
}

std::vector<uint8_t> BWTCompressor::inverseMTF(const std::vector<uint8_t>& input) {
    std::vector<uint8_t> list(256);
    std::iota(list.begin(), list.end(), 0);

    std::vector<uint8_t> output;
    output.reserve(input.size());

    for (uint8_t pos : input) {
        uint8_t b = list[pos];
        output.push_back(b);

        // Move to front
        list.erase(list.begin() + pos);
        list.insert(list.begin(), b);
    }

    return output;
}

std::vector<uint8_t> BWTCompressor::encodeRLE(const std::vector<uint8_t>& input) {
    std::vector<uint8_t> output;
    output.reserve(input.size());

    size_t i = 0;
    while (i < input.size()) {
        if (input[i] == 0) {
            size_t run = 0;
            while (i < input.size() && input[i] == 0 && run < 255) {
                run++;
                i++;
            }
            output.push_back(0);
            output.push_back(static_cast<uint8_t>(run));
        } else {
            output.push_back(input[i]);
            i++;
        }
    }

    return output;
}

std::vector<uint8_t> BWTCompressor::decodeRLE(const std::vector<uint8_t>& input, size_t expectedSize) {
    std::vector<uint8_t> output;
    output.reserve(expectedSize);

    size_t i = 0;
    while (i < input.size() && output.size() < expectedSize) {
        if (input[i] == 0) {
            if (i + 1 >= input.size()) {
                throw std::runtime_error("Corrupted RLE stream: trailing zero marker");
            }
            uint8_t run = input[i + 1];
            output.insert(output.end(), run, 0);
            i += 2;
        } else {
            output.push_back(input[i]);
            i++;
        }
    }

    return output;
}

CompressionMetrics BWTCompressor::compress(std::istream& in, std::ostream& out) {
    auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<uint8_t> inputData(
        (std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>()
    );

    uint64_t originalSize = inputData.size();

    Header header;
    header.algorithm = static_cast<uint8_t>(Algorithm::BWT_MTF);
    header.originalSize = originalSize;
    header.write(out);

    if (originalSize > 0) {
        BWTResult bwt = forwardBWT(inputData);
        std::vector<uint8_t> mtf = forwardMTF(bwt.transformed);
        std::vector<uint8_t> rle = encodeRLE(mtf);

        // Write primary index (4 bytes, little-endian)
        for (int i = 0; i < 4; ++i) {
            out.put(static_cast<char>((bwt.primaryIndex >> (i * 8)) & 0xFF));
        }

        // Write RLE data
        out.write(reinterpret_cast<const char*>(rle.data()), rle.size());
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double durationMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    std::streampos endPos = out.tellp();
    uint64_t compressedSize = (endPos >= 0) ? static_cast<uint64_t>(endPos) : 0;

    CompressionMetrics metrics;
    metrics.algorithm = "bwt";
    metrics.originalSizeBytes = originalSize;
    metrics.compressedSizeBytes = compressedSize;
    metrics.compressionRatio = (compressedSize > 0) ? (static_cast<double>(originalSize) / static_cast<double>(compressedSize)) : 1.0;
    metrics.spaceSavedPercent = (originalSize > 0) ? ((1.0 - static_cast<double>(compressedSize) / static_cast<double>(originalSize)) * 100.0) : 0.0;
    metrics.executionTimeMs = durationMs;
    metrics.success = true;

    return metrics;
}

void BWTCompressor::decompress(std::istream& in, std::ostream& out, uint64_t originalSize) {
    if (originalSize == 0) {
        return;
    }

    uint32_t primaryIndex = 0;
    for (int i = 0; i < 4; ++i) {
        char ch;
        if (!in.get(ch)) {
            throw std::runtime_error("Failed to read BWT primary index");
        }
        primaryIndex |= (static_cast<uint32_t>(static_cast<uint8_t>(ch)) << (i * 8));
    }

    std::vector<uint8_t> rleData(
        (std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>()
    );

    std::vector<uint8_t> mtf = decodeRLE(rleData, originalSize);
    std::vector<uint8_t> bwtTransformed = inverseMTF(mtf);
    std::vector<uint8_t> original = inverseBWT(bwtTransformed, primaryIndex);

    out.write(reinterpret_cast<const char*>(original.data()), original.size());
}

} // namespace adsc
