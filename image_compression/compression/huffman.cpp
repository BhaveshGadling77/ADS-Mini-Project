#include "huffman.h"

#include <queue>
#include <vector>
#include <string>
#include <utility>
#include <stdexcept>

using namespace std;

HuffmanNode *createNode(int data, int freq)
{
    HuffmanNode *node = new HuffmanNode();
    node->data = data;
    node->freq = freq;
    node->left = nullptr;
    node->right = nullptr;
    return node;
}

void destroyTree(HuffmanNode *root)
{
    if (root == nullptr)
        return;
    destroyTree(root->left);
    destroyTree(root->right);
    delete root;
}

struct Compare
{
    bool operator()(HuffmanNode *a, HuffmanNode *b)
    {
        return a->freq > b->freq;
    }
};

void generateCodes(HuffmanNode *root, vector<string> &codes, string str)
{
    if (root == nullptr)
        return;
    if (root->left == nullptr && root->right == nullptr)
    {
        codes[root->data] = str;
        return;
    }
    generateCodes(root->left, codes, str + "0");
    generateCodes(root->right, codes, str + "1");
}

HuffmanNode *buildHuffmanTree(const vector<pair<int, int>> &freq)
{
    if (freq.empty())
        return nullptr;

    priority_queue<HuffmanNode *, vector<HuffmanNode *>, Compare> minHeap;
    for (size_t i = 0; i < freq.size(); i++)
    {
        minHeap.push(createNode(freq[i].first, freq[i].second));
    }

    // Single-symbol case: create a parent so the leaf gets code "0"
    if (minHeap.size() == 1)
    {
        HuffmanNode *leaf = minHeap.top();
        minHeap.pop();
        HuffmanNode *parent = createNode('\0', leaf->freq);
        parent->left = leaf;
        // right stays nullptr — generateCodes will only descend left
        return parent;
    }

    while (minHeap.size() > 1)
    {
        HuffmanNode *left = minHeap.top();
        minHeap.pop();
        HuffmanNode *right = minHeap.top();
        minHeap.pop();
        HuffmanNode *parent = createNode('\0', left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        minHeap.push(parent);
    }
    return minHeap.top();
}

void huffmanEncoding(const vector<pair<int, int>> &freq, vector<string> &codes)
{
    if (freq.empty())
        return;

    HuffmanNode *root = buildHuffmanTree(freq);
    generateCodes(root, codes, "");
    destroyTree(root);
}