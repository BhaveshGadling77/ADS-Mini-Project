#include "decoder.h"
using namespace std;
#include "../format/file_header.h"
#include <fcntl.h>
#include <stdexcept>
#include "huffman.h"
#include "bit_reader.h"
#include <iostream>
#include "../image/bmp_writer.h"
#include "../image/bmp_reader.h"

void decompressImage(
    const char *inputFilename,
    const char *outputFilename)
{
    // 1. Read header
    FileHeader header;
    int fd = open(inputFilename, O_RDONLY);
    if (fd == -1)
        throw runtime_error("Failed to open compressed file");
    header = readFileHeader(fd);

    // 2. Read frequency table
    int uniqueSymbols = header.uniqueSymbols;
    vector<pair<int, int>> freq = readFrequencyTable(fd, uniqueSymbols);

    // 3. Reconstruct Huffman information
    HuffmanNode *huffmanTree = buildHuffmanTree(freq);

    // 4. BitReader reads bits

    // 5. Decode bits
    vector<unsigned char> decodedData;
    ReadBuffer readBuffer{0, 0, 8}; // Initialize ReadBuffer with
    HuffmanNode *currentNode = huffmanTree;

    while(decodedData.size() < header.originalSize){
        int bit = readBit(fd, readBuffer);
        if(bit == -1){
            throw runtime_error("Unexpected end of file while reading bits");
        }
        if(bit == 0){
            currentNode = currentNode->left;
        } else {
            currentNode = currentNode->right;
        }
        if(currentNode->left == nullptr && currentNode->right == nullptr) {
            decodedData.push_back(static_cast<unsigned char>(currentNode->data));
            currentNode = huffmanTree; // Reset to root for next symbol
        }
    }
    if (decodedData.size() != header.originalSize)
    {
        throw runtime_error("Decoded data size mismatch");
    }

    if (decodedData.size() !=
        static_cast<size_t>(header.width) * header.height * 3)
    {
        throw runtime_error("Invalid decoded image data size");
    }

    // 6. Convert to pixels

    vector<Pixel> pixels;
    for (size_t i = 0; i < decodedData.size(); i += 3){
        Pixel pixel;
        pixel.r = decodedData[i];
        pixel.g = decodedData[i + 1];
        pixel.b = decodedData[i + 2];
        pixels.push_back(pixel);
    }

    // 7. Write BMP file
    BMPImage image;
    image.width = header.width;
    image.height = header.height;
    image.pixels = pixels;


    writeBMP(outputFilename, image);
}