#include <bits/stdc++.h>
using namespace std;
#include "huffman.h"

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
    priority_queue<HuffmanNode *, vector<HuffmanNode *>, Compare> minHeap;
    for (int i = 0; i < freq.size(); i++)
    {
        minHeap.push(createNode(freq[i].first, freq[i].second));
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
    HuffmanNode *root = buildHuffmanTree(freq);
    // Generate the codes for each character
    generateCodes(root, codes, "");
    destroyTree(root);
}