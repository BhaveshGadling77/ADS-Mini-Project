#include "io/BitStream.hpp"

namespace adsc {

BitWriter::BitWriter(std::ostream& out) : out(out) {}

BitWriter::~BitWriter() {
    flush();
}

void BitWriter::writeBit(bool bit) {
    buffer = (buffer << 1) | (bit ? 1 : 0);
    bitsInBuffer++;

    if (bitsInBuffer == 8) {
        out.put(static_cast<char>(buffer & 0xFF));
        bytesWritten++;
        buffer = 0;
        bitsInBuffer = 0;
    }
}

void BitWriter::writeBits(uint32_t value, int numBits) {
    if (numBits <= 0) return;
    for (int i = numBits - 1; i >= 0; --i) {
        writeBit((value >> i) & 1);
    }
}

void BitWriter::writeByte(uint8_t byte) {
    writeBits(byte, 8);
}

void BitWriter::flush() {
    if (bitsInBuffer > 0) {
        buffer = buffer << (8 - bitsInBuffer);
        out.put(static_cast<char>(buffer & 0xFF));
        bytesWritten++;
        buffer = 0;
        bitsInBuffer = 0;
    }
    out.flush();
}

BitReader::BitReader(std::istream& in) : in(in) {}

bool BitReader::readBit(bool& bit) {
    if (bitsInBuffer == 0) {
        char ch;
        if (!in.get(ch)) {
            eofReached = true;
            return false;
        }
        buffer = static_cast<uint8_t>(ch);
        bitsInBuffer = 8;
    }

    bitsInBuffer--;
    bit = ((buffer >> bitsInBuffer) & 1) != 0;
    return true;
}

bool BitReader::readBits(int numBits, uint32_t& value) {
    value = 0;
    for (int i = 0; i < numBits; ++i) {
        bool bit = false;
        if (!readBit(bit)) {
            return false;
        }
        value = (value << 1) | (bit ? 1 : 0);
    }
    return true;
}

bool BitReader::readByte(uint8_t& byte) {
    uint32_t val = 0;
    if (!readBits(8, val)) {
        return false;
    }
    byte = static_cast<uint8_t>(val);
    return true;
}

bool BitReader::isEOF() const {
    return eofReached;
}

} // namespace adsc
