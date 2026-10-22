#include "encoder.h"

#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <stdexcept>
#include <cerrno>

#include "../image/bmp_reader.h"
#include "data_preprocessor.h"
#include "frequency.h"
#include "algorithms/huffman/HuffmanCodec.hpp"
#include "../format/file_header.h"
#include "bit_writer.h"
#include "spatial_predictor.h"

using namespace std;

// RAII wrapper for file descriptors
class FdGuard
{
public:
    explicit FdGuard(int fd) : fd_(fd) {}
    ~FdGuard()
    {
        if (fd_ >= 0)
            close(fd_);
    }
    FdGuard(const FdGuard &) = delete;
    FdGuard &operator=(const FdGuard &) = delete;
    int get() const { return fd_; }
    int release()
    {
        int fd = fd_;
        fd_ = -1;
        return fd;
    }

private:
    int fd_;
};

void compressImage(
    const char *inputFilename,
    const char *outputFilename)
{
    // 1. Read image
    BMPImage image = readImage(inputFilename);

    cout << "Width: " << image.width << "\n";
    cout << "Height: " << image.height << "\n";
    cout << "Pixels: " << image.pixels.size() << "\n";

    for (size_t i = 0; i < 5 && i < image.pixels.size(); i++)
    {
        cout << "Pixel " << i << ": "
             << (int)image.pixels[i].r << " "
             << (int)image.pixels[i].g << " "
             << (int)image.pixels[i].b << "\n";
    }

    // 2. Prepare byte data
    vector<unsigned char> data = prepareData(image);

    // 2b. Apply left-neighbor spatial prediction to R, G, B channels
    applySpatialPrediction(data, image.width, image.height);

    cout << "Data bytes: " << data.size() << "\n";

    for (size_t i = 0; i < 15 && i < data.size(); i++)
    {
        cout << (int)data[i] << " ";
    }

    cout << "\n";

    // 3. Calculate frequencies
    vector<pair<int, int>> freq = calculateFrequency(data);

    cout << "Unique bytes: " << freq.size() << "\n";

    for (const auto &entry : freq)
    {
        cout << entry.first << " -> " << entry.second << "\n";
    }

    // 4. Generate Huffman codes
    adsc::HuffmanCodec::Frequencies huffmanFrequencies{};
    for (const auto &entry : freq)
    {
        huffmanFrequencies[entry.first] = static_cast<uint64_t>(entry.second);
    }
    const auto huffmanTree = adsc::HuffmanCodec::buildTree(huffmanFrequencies);
    const auto codes = adsc::HuffmanCodec::buildCodeTable(huffmanTree);

    cout << "\nHuffman Codes:\n";

    for (const auto &p : freq)
    {
        cout << p.first << " -> ";
        for (bool bit : codes[p.first])
            cout << (bit ? '1' : '0');
        cout << "\n";
    }

    // 5. Prepare compressed file header
    FileHeader header{};

    header.magic[0] = 'H';
    header.magic[1] = 'U';
    header.magic[2] = 'F';
    header.magic[3] = '2';

    header.width = image.width;
    header.height = image.height;
    header.originalSize = static_cast<int>(data.size());
    header.validBits = 0; // will be updated after writing encoded data
    header.uniqueSymbols = static_cast<int>(freq.size());

    // 6. Open compressed file
    int fd = open(
        outputFilename,
        O_WRONLY | O_CREAT | O_TRUNC,
        0644);

    if (fd == -1)
    {
        throw runtime_error("Failed to open output file");
    }

    FdGuard fdGuard(fd);

    // 7. Write header
    writeFileHeader(fd, header);

    // 8. Write frequency table
    writeFrequencyTable(fd, freq);

    // 9. Write Huffman encoded data
    vector<int> encodedData(data.begin(), data.end());
    int validBits = BitWriter(fd, codes, encodedData);

    // 10. Update validBits in the header
    header.validBits = validBits;
    off_t validBitsOffset = offsetof(FileHeader, validBits);
    if (lseek(fd, validBitsOffset, SEEK_SET) == -1)
    {
        throw runtime_error("Failed to seek to update validBits");
    }

    ssize_t n = write(fd, &header.validBits, sizeof(header.validBits));
    if (n != sizeof(header.validBits))
    {
        throw runtime_error("Failed to write validBits");
    }

    cout << "Valid bits: " << header.validBits << "\n";

    // 11. FdGuard closes fd automatically
}
