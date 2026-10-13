#include "algorithms/text/lzw/LZWCompressor.hpp"
#include "core/FileHeader.hpp"
#include "io/BitStream.hpp"
#include <chrono>
#include <vector>
#include <iostream>

namespace adsc {

LZWCompressor::LZWCompressor() = default;

CompressionMetrics LZWCompressor::compress(std::istream& in, std::ostream& out) {
    auto startTime = std::chrono::high_resolution_clock::now();

    // Read full input into memory to determine size and fast compression
    std::vector<uint8_t> inputData(
        (std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>()
    );

    uint64_t originalSize = inputData.size();

    // Write file header
    Header header;
    header.algorithm = static_cast<uint8_t>(Algorithm::LZW);
    header.originalSize = originalSize;
    header.write(out);

    if (originalSize > 0) {
        LzwTrie trie;
        BitWriter writer(out);

        int currentBits = INIT_BITS;
        uint32_t nextCode = 256;

        TrieNode* curr = trie.getRoot()->findChild(inputData[0]);

        for (size_t i = 1; i < inputData.size(); ++i) {
            uint8_t byte = inputData[i];
            TrieNode* child = trie.findChild(curr, byte);

            if (child != nullptr) {
                curr = child;
            } else {
                writer.writeBits(static_cast<uint32_t>(curr->code), currentBits);

                if (nextCode < MAX_CODES) {
                    trie.insertChild(curr, byte, nextCode++);
                    if (nextCode >= (1U << currentBits) && currentBits < MAX_BITS) {
                        currentBits++;
                    }
                }

                curr = trie.getRoot()->findChild(byte);
            }
        }

        if (curr != nullptr) {
            writer.writeBits(static_cast<uint32_t>(curr->code), currentBits);
        }

        writer.flush();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double durationMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    // Get current output stream position to calculate compressed size
    std::streampos endPos = out.tellp();
    uint64_t compressedSize = (endPos >= 0) ? static_cast<uint64_t>(endPos) : 0;

    CompressionMetrics metrics;
    metrics.algorithm = "lzw";
    metrics.originalSizeBytes = originalSize;
    metrics.compressedSizeBytes = compressedSize;
    metrics.compressionRatio = (compressedSize > 0) ? (static_cast<double>(originalSize) / static_cast<double>(compressedSize)) : 1.0;
    metrics.spaceSavedPercent = (originalSize > 0) ? ((1.0 - static_cast<double>(compressedSize) / static_cast<double>(originalSize)) * 100.0) : 0.0;
    metrics.executionTimeMs = durationMs;
    metrics.success = true;

    return metrics;
}

void LZWCompressor::decompress(std::istream& in, std::ostream& out, uint64_t originalSize) {
    if (originalSize == 0) {
        return;
    }

    std::vector<std::vector<uint8_t>> dictionary;
    dictionary.reserve(MAX_CODES + 1);
    for (int i = 0; i < 256; ++i) {
        dictionary.push_back({static_cast<uint8_t>(i)});
    }

    BitReader reader(in);
    int currentBits = INIT_BITS;
    uint32_t nextCode = 256;

    uint32_t prevCode = 0;
    if (!reader.readBits(currentBits, prevCode)) {
        return;
    }

    if (prevCode >= dictionary.size()) {
        throw std::runtime_error("Corrupted LZW stream: invalid initial code");
    }

    std::vector<uint8_t> currentSeq = dictionary[prevCode];
    out.write(reinterpret_cast<const char*>(currentSeq.data()), currentSeq.size());
    uint64_t bytesWritten = currentSeq.size();

    uint32_t code = 0;
    while (bytesWritten < originalSize && reader.readBits(currentBits, code)) {
        std::vector<uint8_t> entry;

        if (code < dictionary.size()) {
            entry = dictionary[code];
        } else if (code == dictionary.size() && !currentSeq.empty()) {
            // Special LZW case: cScSc pattern
            entry = currentSeq;
            entry.push_back(currentSeq[0]);
        } else {
            std::string msg = "Corrupted LZW stream: invalid code " + std::to_string(code)
                + " (dict.size=" + std::to_string(dictionary.size())
                + ", nextCode=" + std::to_string(nextCode)
                + ", currentBits=" + std::to_string(currentBits)
                + ", bytesWritten=" + std::to_string(bytesWritten)
                + "/" + std::to_string(originalSize) + ")";
            throw std::runtime_error(msg);
        }

        out.write(reinterpret_cast<const char*>(entry.data()), entry.size());
        bytesWritten += entry.size();

        if (nextCode < MAX_CODES && !currentSeq.empty()) {
            std::vector<uint8_t> newEntry = currentSeq;
            newEntry.push_back(entry[0]);
            dictionary.push_back(std::move(newEntry));
            nextCode++;

            if (nextCode + 1 >= (1U << currentBits) && currentBits < MAX_BITS) {
                currentBits++;
            }
        }

        currentSeq = entry;
    }
}

} // namespace adsc
