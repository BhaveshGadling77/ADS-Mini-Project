#pragma once

#include <iostream>
#include <cstdint>
#include <stdexcept>

namespace adsc {

class BitWriter {
public:
    explicit BitWriter(std::ostream& out);
    ~BitWriter();

    void writeBit(bool bit);
    void writeBits(uint32_t value, int numBits);
    void writeByte(uint8_t byte);
    void flush();

    uint64_t getBytesWritten() const { return bytesWritten; }

private:
    std::ostream& out;
    uint32_t buffer = 0;
    int bitsInBuffer = 0;
    uint64_t bytesWritten = 0;
};

class BitReader {
public:
    explicit BitReader(std::istream& in);

    bool readBit(bool& bit);
    bool readBits(int numBits, uint32_t& value);
    bool readByte(uint8_t& byte);
    bool isEOF() const;

private:
    std::istream& in;
    uint32_t buffer = 0;
    int bitsInBuffer = 0;
    bool eofReached = false;
};

} // namespace adsc
