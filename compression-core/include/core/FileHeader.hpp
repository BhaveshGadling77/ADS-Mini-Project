#pragma once

#include "core/Types.hpp"
#include <cstdint>
#include <iostream>
#include <array>
#include <stdexcept>

namespace adsc {

constexpr std::array<char, 4> MAGIC_BYTES = {'A', 'D', 'S', 'C'};
constexpr uint8_t FORMAT_VERSION = 1;
constexpr uint8_t MODALITY_TEXT = 1;

struct Header {
    std::array<char, 4> magic = MAGIC_BYTES;
    uint8_t version = FORMAT_VERSION;
    uint8_t algorithm = 0;
    uint8_t modality = MODALITY_TEXT;
    uint8_t flags = 0;
    uint64_t originalSize = 0;

    void write(std::ostream& out) const {
        out.write(magic.data(), 4);
        out.put(static_cast<char>(version));
        out.put(static_cast<char>(algorithm));
        out.put(static_cast<char>(modality));
        out.put(static_cast<char>(flags));
        
        for (int i = 0; i < 8; ++i) {
            out.put(static_cast<char>((originalSize >> (i * 8)) & 0xFF));
        }
    }

    static Header read(std::istream& in) {
        Header hdr;
        if (!in.read(hdr.magic.data(), 4)) {
            throw std::runtime_error("Failed to read magic bytes from header");
        }
        if (hdr.magic != MAGIC_BYTES) {
            throw std::runtime_error("Invalid file format: magic bytes do not match 'ADSC'");
        }
        char b;
        if (!in.get(b)) throw std::runtime_error("Failed to read version");
        hdr.version = static_cast<uint8_t>(b);
        if (hdr.version != FORMAT_VERSION) {
            throw std::runtime_error("Unsupported format version: " + std::to_string(hdr.version));
        }
        if (!in.get(b)) throw std::runtime_error("Failed to read algorithm ID");
        hdr.algorithm = static_cast<uint8_t>(b);
        if (!in.get(b)) throw std::runtime_error("Failed to read modality");
        hdr.modality = static_cast<uint8_t>(b);
        if (!in.get(b)) throw std::runtime_error("Failed to read flags");
        hdr.flags = static_cast<uint8_t>(b);

        hdr.originalSize = 0;
        for (int i = 0; i < 8; ++i) {
            if (!in.get(b)) throw std::runtime_error("Failed to read original size");
            hdr.originalSize |= (static_cast<uint64_t>(static_cast<uint8_t>(b)) << (i * 8));
        }
        return hdr;
    }
};

} // namespace adsc
