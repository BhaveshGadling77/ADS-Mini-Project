#ifndef BIT_WRITER_H
#define BIT_WRITER_H

#include "algorithms/huffman/HuffmanCodec.hpp"

#include <vector>

struct Buffer {
    unsigned char byte;
    int bitCount;
};

// Writes Huffman-encoded bits for each symbol in data.
// Returns the number of valid bits in the last byte (1-8), or 0 if no bits were written.
// Does NOT close fd — caller retains ownership.
int BitWriter(int fd, const adsc::HuffmanCodec::CodeTable &codes, const std::vector<int> &data);

#endif
