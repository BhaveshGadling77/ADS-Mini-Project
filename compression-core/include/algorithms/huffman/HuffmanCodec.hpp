#pragma once

#include "datastructures/heap/MinHeap.hpp"
#include "datastructures/tree/BinaryTree.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace adsc {

// Shared Huffman tree and code-table implementation used by each byte-oriented
// modality. Container headers and bitstream I/O remain owned by each format.
class HuffmanCodec {
public:
    using Frequencies = std::array<uint64_t, 256>;
    using Code = std::vector<bool>;
    using CodeTable = std::array<Code, 256>;
    using Node = HuffmanNode;
    using NodePtr = std::shared_ptr<Node>;

    static NodePtr buildTree(const Frequencies& frequencies) {
        MinHeap<NodePtr, HuffmanNodeCompare> heap;

        for (std::size_t symbol = 0; symbol < frequencies.size(); ++symbol) {
            if (frequencies[symbol] != 0) {
                heap.push(std::make_shared<Node>(
                    static_cast<uint8_t>(symbol), frequencies[symbol]));
            }
        }

        if (heap.empty()) {
            return nullptr;
        }

        if (heap.size() == 1) {
            NodePtr leaf = heap.top();
            heap.pop();
            return std::make_shared<Node>(leaf->frequency, leaf, nullptr);
        }

        while (heap.size() > 1) {
            NodePtr left = heap.top();
            heap.pop();
            NodePtr right = heap.top();
            heap.pop();
            heap.push(std::make_shared<Node>(
                left->frequency + right->frequency, left, right));
        }

        return heap.top();
    }

    static CodeTable buildCodeTable(const NodePtr& root) {
        CodeTable table;
        Code current;
        buildCodes(root, current, table);
        return table;
    }

private:
    static void buildCodes(const NodePtr& node, Code& current, CodeTable& table) {
        if (!node) {
            return;
        }

        if (node->isLeaf()) {
            // Assign a real bit to the one-symbol alphabet so formats that
            // serialize a bitstream can encode repeated copies unambiguously.
            if (current.empty()) {
                table[node->byteValue] = Code{false};
            } else {
                table[node->byteValue] = current;
            }
            return;
        }

        if (node->left) {
            current.push_back(false);
            buildCodes(node->left, current, table);
            current.pop_back();
        }
        if (node->right) {
            current.push_back(true);
            buildCodes(node->right, current, table);
            current.pop_back();
        }
    }
};

} // namespace adsc
