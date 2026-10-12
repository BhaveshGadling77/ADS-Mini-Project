#pragma once

#include "core/Types.hpp"
#include <iostream>

namespace adsc {

class ICompressor {
public:
    virtual ~ICompressor() = default;

    virtual Algorithm getAlgorithm() const = 0;
    virtual std::string getName() const = 0;

    // Compresses data from input stream to output stream, writing file header + compressed payload
    virtual CompressionMetrics compress(std::istream& in, std::ostream& out) = 0;

    // Decompresses payload from input stream (after header has been read) to output stream
    virtual void decompress(std::istream& in, std::ostream& out, uint64_t originalSize) = 0;
};

} // namespace adsc
