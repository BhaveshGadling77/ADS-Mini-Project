#include "decoder.h"

#include "../format/file_header.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include "huffman.h"
#include "bit_reader.h"
#include <iostream>
#include "../image/bmp_writer.h"
#include "../image/bmp_reader.h"
#include "spatial_predictor.h"

using namespace std;

// RAII wrapper for file descriptors
class DecoderFdGuard
{
public:
    explicit DecoderFdGuard(int fd) : fd_(fd) {}
    ~DecoderFdGuard()
    {
        if (fd_ >= 0)
            close(fd_);
    }
    DecoderFdGuard(const DecoderFdGuard &) = delete;
    DecoderFdGuard &operator=(const DecoderFdGuard &) = delete;
    int get() const { return fd_; }

private:
    int fd_;
};

// RAII wrapper for Huffman trees
class TreeGuard
{
public:
    explicit TreeGuard(HuffmanNode *root) : root_(root) {}
    ~TreeGuard() { destroyTree(root_); }
    TreeGuard(const TreeGuard &) = delete;
    TreeGuard &operator=(const TreeGuard &) = delete;
    HuffmanNode *get() const { return root_; }

private:
    HuffmanNode *root_;
};

void decompressImage(
    const char *inputFilename,
    const char *outputFilename)
{
    // 1. Open and read header
    int fd = open(inputFilename, O_RDONLY);
    if (fd == -1)
        throw runtime_error("Failed to open compressed file");

    DecoderFdGuard fdGuard(fd);

    FileHeader header = readFileHeader(fd);
    bool useSpatialPrediction = isSpatialPredictionHeader(header);

    // 2. Read frequency table
    int uniqueSymbols = header.uniqueSymbols;
    vector<pair<int, int>> freq = readFrequencyTable(fd, uniqueSymbols);

    // 3. Validate expected data size
    size_t expectedSize = static_cast<size_t>(header.originalSize);
    size_t expectedPixelBytes = static_cast<size_t>(header.width) * header.height * 3;

    if (expectedSize != expectedPixelBytes)
    {
        throw runtime_error("Header originalSize does not match width*height*3");
    }

    // 4. Handle empty data
    if (expectedSize == 0)
    {
        BMPImage image;
        image.width = header.width;
        image.height = header.height;
        writeBMP(outputFilename, image);
        return;
    }

    // 5. Reconstruct Huffman tree
    HuffmanNode *huffmanTree = buildHuffmanTree(freq);
    TreeGuard treeGuard(huffmanTree);

    if (huffmanTree == nullptr)
    {
        throw runtime_error("Failed to build Huffman tree from frequency table");
    }

    // 6. Decode bits
    vector<unsigned char> decodedData;
    decodedData.reserve(expectedSize);

    // Single-symbol case: root has a left child (the leaf) but we still
    // need to read the encoded bits. The encoder writes code "0" for each symbol.
    bool isSingleSymbol = (huffmanTree->left != nullptr &&
                           huffmanTree->right == nullptr &&
                           huffmanTree->left->left == nullptr &&
                           huffmanTree->left->right == nullptr);

    ReadBuffer readBuffer{0, 0, 8};

    if (isSingleSymbol)
    {
        unsigned char symbol = static_cast<unsigned char>(huffmanTree->left->data);
        // Each symbol was encoded as a single "0" bit.
        // We need to read the correct number of bits to consume the stream,
        // but we already know every decoded symbol is the same.
        for (size_t i = 0; i < expectedSize; i++)
        {
            int bit = readBit(fd, readBuffer);
            if (bit == -1)
            {
                throw runtime_error("Unexpected EOF reading single-symbol data");
            }
            // bit should be 0 (the code for the only symbol)
            decodedData.push_back(symbol);
        }
    }
    else
    {
        HuffmanNode *currentNode = huffmanTree;

        while (decodedData.size() < expectedSize)
        {
            int bit = readBit(fd, readBuffer);
            if (bit == -1)
            {
                throw runtime_error("Unexpected end of file while reading bits");
            }
            if (bit == 0)
            {
                currentNode = currentNode->left;
            }
            else
            {
                currentNode = currentNode->right;
            }

            if (currentNode == nullptr)
            {
                throw runtime_error("Invalid Huffman tree traversal (null node)");
            }

            if (currentNode->left == nullptr && currentNode->right == nullptr)
            {
                decodedData.push_back(static_cast<unsigned char>(currentNode->data));
                currentNode = huffmanTree; // Reset to root for next symbol
            }
        }
    }

    if (decodedData.size() != expectedSize)
    {
        throw runtime_error("Decoded data size mismatch");
    }

    // 6b. Reverse spatial prediction if HUF2 format was used
    if (useSpatialPrediction)
    {
        applyInverseSpatialPrediction(decodedData, header.width, header.height);
    }

    // 7. Convert to pixels
    vector<Pixel> pixels;
    pixels.reserve(static_cast<size_t>(header.width) * header.height);

    for (size_t i = 0; i < decodedData.size(); i += 3)
    {
        Pixel pixel;
        pixel.r = decodedData[i];
        pixel.g = decodedData[i + 1];
        pixel.b = decodedData[i + 2];
        pixels.push_back(pixel);
    }

    // 8. Write BMP file
    BMPImage image;
    image.width = header.width;
    image.height = header.height;
    image.pixels = pixels;

    writeBMP(outputFilename, image);
}