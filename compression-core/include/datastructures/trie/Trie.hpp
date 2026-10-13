#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <memory>

namespace adsc {

class TrieNode {
public:
    int32_t code;
    std::unordered_map<uint8_t, std::unique_ptr<TrieNode>> children;

    explicit TrieNode(int32_t code = -1) : code(code) {}

    TrieNode* findChild(uint8_t byte) const {
        auto it = children.find(byte);
        if (it != children.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    TrieNode* insertChild(uint8_t byte, int32_t newCode) {
        auto child = std::make_unique<TrieNode>(newCode);
        TrieNode* ptr = child.get();
        children[byte] = std::move(child);
        return ptr;
    }
};

class LzwTrie {
public:
    LzwTrie() {
        init();
    }

    void init() {
        root = std::make_unique<TrieNode>(-1);
        for (int i = 0; i < 256; ++i) {
            root->insertChild(static_cast<uint8_t>(i), i);
        }
    }

    void clear() {
        root.reset();
        init();
    }

    TrieNode* getRoot() const {
        return root.get();
    }

    TrieNode* findChild(TrieNode* node, uint8_t byte) const {
        if (!node) return nullptr;
        return node->findChild(byte);
    }

    TrieNode* insertChild(TrieNode* node, uint8_t byte, int32_t code) {
        if (!node) return nullptr;
        return node->insertChild(byte, code);
    }

private:
    std::unique_ptr<TrieNode> root;
};

} // namespace adsc
