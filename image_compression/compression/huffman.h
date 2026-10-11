#ifndef HUFFMAN_H
#define HUFFMAN_H
#include <vector>
#include <string>
#include <utility>


typedef struct HuffmanNode {
  int data; // pixel value (0-255)
  int freq;
  struct HuffmanNode *left;
  struct HuffmanNode *right;
} HuffmanNode;

HuffmanNode *createNode(int data, int freq);
void destroyTree(HuffmanNode *root);
HuffmanNode *buildHuffmanTree(const std::vector<std::pair<int, int>> &freq);
void huffmanEncoding(const std::vector<std::pair<int, int>> &freq, std::vector<std::string> &codes);

#endif // HUFFMAN_H