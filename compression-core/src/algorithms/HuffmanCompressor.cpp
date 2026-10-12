#include "algorithms/text/huffman/HuffmanCompressor.hpp"
#include "core/FileHeader.hpp"
#include "io/BitStream.hpp"
#include <chrono>
#include <iostream>

namespace adsc {

HuffmanCompressor::HuffmanCompressor() = default;

std::shared_ptr<HuffmanNode> HuffmanCompressor::buildTree(const std::vector<uint64_t>& frequencies) {
    MinHeap<std::shared_ptr<HuffmanNode>, HuffmanNodeCompare> heap;

    for (int i = 0; i < 256; ++i) {
        if (frequencies[i] > 0) {
            heap.push(std::make_shared<HuffmanNode>(static_cast<uint8_t>(i), frequencies[i]));
        }
    }

    if (heap.empty()) {
        return nullptr;
    }

    if (heap.size() == 1) {
        auto singleNode = heap.top();
        heap.pop();
        auto parent = std::make_shared<HuffmanNode>(singleNode->frequency, singleNode, nullptr);
        return parent;
    }

    while (heap.size() > 1) {
        auto left = heap.top();
        heap.pop();
        auto right = heap.top();
        heap.pop();

        auto parent = std::make_shared<HuffmanNode>(left->frequency + right->frequency, left, right);
        heap.push(parent);
    }

    return heap.top();
}

void HuffmanCompressor::generateCodes(
    const std::shared_ptr<HuffmanNode>& node,
    std::vector<bool>& currentBits,
    std::vector<Code>& table) {
    if (!node) return;

    if (node->isLeaf()) {
        if (currentBits.empty()) {
            table[node->byteValue] = Code{{false}};
        } else {
            table[node->byteValue] = Code{currentBits};
        }
        return;
    }

    if (node->left) {
        currentBits.push_back(false);
        generateCodes(node->left, currentBits, table);
        currentBits.pop_back();
    }

    if (node->right) {
        currentBits.push_back(true);
        generateCodes(node->right, currentBits, table);
        currentBits.pop_back();
    }
}

CompressionMetrics HuffmanCompressor::compress(std::istream& in, std::ostream& out) {
    auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<uint8_t> inputData(
        (std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>()
    );

    uint64_t originalSize = inputData.size();

    Header header;
    header.algorithm = static_cast<uint8_t>(Algorithm::HUFFMAN);
    header.originalSize = originalSize;
    header.write(out);

    if (originalSize > 0) {
        std::vector<uint64_t> frequencies(256, 0);
        for (uint8_t b : inputData) {
            frequencies[b]++;
        }

        uint16_t nonZeroCount = 0;
        for (int i = 0; i < 256; ++i) {
            if (frequencies[i] > 0) nonZeroCount++;
        }

        // Write frequency table header
        out.put(static_cast<char>(nonZeroCount & 0xFF));
        out.put(static_cast<char>((nonZeroCount >> 8) & 0xFF));

        for (int i = 0; i < 256; ++i) {
            if (frequencies[i] > 0) {
                out.put(static_cast<char>(i));
                uint32_t freq32 = static_cast<uint32_t>(frequencies[i]);
                out.put(static_cast<char>(freq32 & 0xFF));
                out.put(static_cast<char>((freq32 >> 8) & 0xFF));
                out.put(static_cast<char>((freq32 >> 16) & 0xFF));
                out.put(static_cast<char>((freq32 >> 24) & 0xFF));
            }
        }

        auto root = buildTree(frequencies);
        std::vector<Code> table(256);
        std::vector<bool> currentBits;
        generateCodes(root, currentBits, table);

        BitWriter writer(out);
        for (uint8_t b : inputData) {
            const auto& bits = table[b].bits;
            for (bool bit : bits) {
                writer.writeBit(bit);
            }
        }
        writer.flush();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double durationMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    std::streampos endPos = out.tellp();
    uint64_t compressedSize = (endPos >= 0) ? static_cast<uint64_t>(endPos) : 0;

    CompressionMetrics metrics;
    metrics.algorithm = "huffman";
    metrics.originalSizeBytes = originalSize;
    metrics.compressedSizeBytes = compressedSize;
    metrics.compressionRatio = (compressedSize > 0) ? (static_cast<double>(originalSize) / static_cast<double>(compressedSize)) : 1.0;
    metrics.spaceSavedPercent = (originalSize > 0) ? ((1.0 - static_cast<double>(compressedSize) / static_cast<double>(originalSize)) * 100.0) : 0.0;
    metrics.executionTimeMs = durationMs;
    metrics.success = true;

    return metrics;
}

void HuffmanCompressor::decompress(std::istream& in, std::ostream& out, uint64_t originalSize) {
    if (originalSize == 0) {
        return;
    }

    char b1, b2;
    if (!in.get(b1) || !in.get(b2)) {
        throw std::runtime_error("Failed to read Huffman table header");
    }
    uint16_t nonZeroCount = static_cast<uint8_t>(b1) | (static_cast<uint16_t>(static_cast<uint8_t>(b2)) << 8);

    std::vector<uint64_t> frequencies(256, 0);
    for (uint16_t i = 0; i < nonZeroCount; ++i) {
        char symbolChar;
        if (!in.get(symbolChar)) {
            throw std::runtime_error("Corrupted Huffman table: missing symbol");
        }
        uint8_t symbol = static_cast<uint8_t>(symbolChar);

        uint32_t freq32 = 0;
        for (int b = 0; b < 4; ++b) {
            char fByte;
            if (!in.get(fByte)) {
                throw std::runtime_error("Corrupted Huffman table: missing frequency byte");
            }
            freq32 |= (static_cast<uint32_t>(static_cast<uint8_t>(fByte)) << (b * 8));
        }
        frequencies[symbol] = freq32;
    }

    auto root = buildTree(frequencies);
    if (!root) {
        throw std::runtime_error("Failed to construct Huffman tree");
    }

    if (root->isLeaf()) {
        std::vector<char> buf(originalSize, static_cast<char>(root->byteValue));
        out.write(buf.data(), buf.size());
        return;
    }

    BitReader reader(in);
    for (uint64_t i = 0; i < originalSize; ++i) {
        auto curr = root;
        while (!curr->isLeaf()) {
            bool bit = false;
            if (!reader.readBit(bit)) {
                throw std::runtime_error("Corrupted Huffman stream: unexpected EOF");
            }
            curr = bit ? curr->right : curr->left;
            if (!curr) {
                throw std::runtime_error("Corrupted Huffman stream: invalid bit sequence");
            }
        }
        out.put(static_cast<char>(curr->byteValue));
    }
}

} // namespace adsc
