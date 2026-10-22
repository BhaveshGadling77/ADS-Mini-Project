// tests/test_roundtrip.cpp
// Automated test suite for the image compression project.
// Generates BMP files programmatically, compresses them, decompresses,
// and verifies pixel-exact round-trip correctness.

#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <fstream>
#include <algorithm>
#include <functional>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>

#include "../image/bmp_reader.h"
#include "../image/bmp_writer.h"
#include "../compression/encoder.h"
#include "../compression/decoder.h"
#include "../compression/frequency.h"
#include "algorithms/huffman/HuffmanCodec.hpp"
#include "../compression/bit_writer.h"
#include "../format/file_header.h"

using namespace std;

// ============================================================================
// Test infrastructure
// ============================================================================

static int g_passed = 0;
static int g_failed = 0;
static int g_total = 0;

#define TEST(name)                                          \
    do                                                      \
    {                                                       \
        g_total++;                                          \
        cout << "TEST " << g_total << ": " << name << " "; \
        cout.flush();                                       \
    } while (0)

#define PASS()                      \
    do                              \
    {                               \
        g_passed++;                 \
        cout << " [PASS]" << endl;  \
    } while (0)

#define FAIL(msg)                                  \
    do                                             \
    {                                              \
        g_failed++;                                \
        cout << " [FAIL] " << msg << endl;         \
    } while (0)

// Temporary file helper
class TempFile
{
public:
    TempFile(const string &suffix)
    {
        path_ = "/tmp/test_compressor_" + to_string(getpid()) + "_" +
                to_string(counter_++) + suffix;
    }
    ~TempFile() { remove(path_.c_str()); }
    const char *path() const { return path_.c_str(); }
    const string &str() const { return path_; }

private:
    string path_;
    static int counter_;
};
int TempFile::counter_ = 0;

// ============================================================================
// BMP generation helpers
// ============================================================================

// Generate a BMPImage with given dimensions and pixel-fill function
BMPImage generateImage(int width, int height,
                       function<Pixel(int x, int y)> pixelFunc)
{
    BMPImage image;
    image.width = width;
    image.height = height;
    image.pixels.resize(static_cast<size_t>(width) * height);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            image.pixels[y * width + x] = pixelFunc(x, y);
        }
    }
    return image;
}

// Compare two BMPImages pixel-by-pixel
bool compareImages(const BMPImage &a, const BMPImage &b, string &errMsg)
{
    if (a.width != b.width || a.height != b.height)
    {
        errMsg = "Dimensions differ: " + to_string(a.width) + "x" + to_string(a.height) +
                 " vs " + to_string(b.width) + "x" + to_string(b.height);
        return false;
    }
    if (a.pixels.size() != b.pixels.size())
    {
        errMsg = "Pixel count differs";
        return false;
    }
    for (size_t i = 0; i < a.pixels.size(); i++)
    {
        if (a.pixels[i].r != b.pixels[i].r ||
            a.pixels[i].g != b.pixels[i].g ||
            a.pixels[i].b != b.pixels[i].b)
        {
            ostringstream oss;
            oss << "Pixel mismatch at index " << i
                << ": (" << (int)a.pixels[i].r << "," << (int)a.pixels[i].g << "," << (int)a.pixels[i].b << ")"
                << " vs (" << (int)b.pixels[i].r << "," << (int)b.pixels[i].g << "," << (int)b.pixels[i].b << ")";
            errMsg = oss.str();
            return false;
        }
    }
    return true;
}

// Full round-trip test: write BMP → compress → decompress → read BMP → compare
bool roundTripTest(const BMPImage &original, string &errMsg)
{
    TempFile bmpFile(".bmp");
    TempFile hufFile(".huf");
    TempFile outFile(".bmp");

    try
    {
        // Write original BMP
        writeBMP(bmpFile.path(), original);

        // Compress
        compressImage(bmpFile.path(), hufFile.path());

        // Decompress
        decompressImage(hufFile.path(), outFile.path());

        // Read back
        BMPImage reconstructed = readImage(outFile.path());

        // Compare
        return compareImages(original, reconstructed, errMsg);
    }
    catch (const exception &e)
    {
        errMsg = string("Exception: ") + e.what();
        return false;
    }
}

// ============================================================================
// Round-trip tests
// ============================================================================

void test_small_rgb()
{
    TEST("Small 4x4 RGB image");
    BMPImage img = generateImage(4, 4, [](int x, int y) -> Pixel
                                 {
        Pixel p;
        p.r = static_cast<unsigned char>((x * 60 + y * 30) % 256);
        p.g = static_cast<unsigned char>((x * 40 + y * 50) % 256);
        p.b = static_cast<unsigned char>((x * 20 + y * 70) % 256);
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_small_rgb_8x6()
{
    TEST("Small 8x6 RGB image");
    BMPImage img = generateImage(8, 6, [](int x, int y) -> Pixel
                                 {
        Pixel p;
        p.r = static_cast<unsigned char>((x + y * 8) % 256);
        p.g = static_cast<unsigned char>((x * 3 + y * 5) % 256);
        p.b = static_cast<unsigned char>((x * 7 + y * 2) % 256);
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_row_padding_width3()
{
    TEST("Row padding: width=3 (row=9 bytes, padded to 12)");
    BMPImage img = generateImage(3, 3, [](int x, int y) -> Pixel
                                 {
        Pixel p;
        p.r = static_cast<unsigned char>(100 + x * 10 + y * 20);
        p.g = static_cast<unsigned char>(50 + x * 15 + y * 25);
        p.b = static_cast<unsigned char>(200 - x * 5 - y * 10);
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_row_padding_width5()
{
    TEST("Row padding: width=5 (row=15 bytes, padded to 16)");
    BMPImage img = generateImage(5, 4, [](int x, int y) -> Pixel
                                 {
        Pixel p;
        p.r = static_cast<unsigned char>((x * 50 + y * 30) % 256);
        p.g = static_cast<unsigned char>((x * 60 + y * 40) % 256);
        p.b = static_cast<unsigned char>((x * 70 + y * 50) % 256);
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_row_padding_width7()
{
    TEST("Row padding: width=7 (row=21 bytes, padded to 24)");
    BMPImage img = generateImage(7, 3, [](int x, int y) -> Pixel
                                 {
        Pixel p;
        p.r = static_cast<unsigned char>((x + y) * 30 % 256);
        p.g = static_cast<unsigned char>((x + y) * 40 % 256);
        p.b = static_cast<unsigned char>((x + y) * 50 % 256);
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_1x1_pixel()
{
    TEST("1x1 single pixel image");
    BMPImage img = generateImage(1, 1, [](int, int) -> Pixel
                                 {
        Pixel p;
        p.r = 128;
        p.g = 64;
        p.b = 200;
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_single_symbol_uniform()
{
    TEST("Single-symbol: uniform red 4x4 image");
    BMPImage img = generateImage(4, 4, [](int, int) -> Pixel
                                 {
        Pixel p;
        p.r = 42;
        p.g = 42;
        p.b = 42;
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_single_symbol_1x1()
{
    TEST("Single-symbol: 1x1 uniform pixel");
    BMPImage img = generateImage(1, 1, [](int, int) -> Pixel
                                 {
        Pixel p;
        p.r = 255;
        p.g = 255;
        p.b = 255;
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_two_symbols()
{
    TEST("Two-symbol: alternating black and white pixels");
    BMPImage img = generateImage(4, 4, [](int x, int y) -> Pixel
                                 {
        Pixel p;
        unsigned char val = ((x + y) % 2 == 0) ? 0 : 255;
        p.r = val;
        p.g = val;
        p.b = val;
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_two_symbols_rgb()
{
    TEST("Two-symbol RGB: red and blue alternating");
    BMPImage img = generateImage(6, 4, [](int x, int) -> Pixel
                                 {
        Pixel p;
        if (x % 2 == 0) {
            p.r = 255; p.g = 0; p.b = 0;
        } else {
            p.r = 0; p.g = 0; p.b = 255;
        }
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_all_256_grayscale_values()
{
    TEST("All 256 grayscale values (16x16 image)");
    BMPImage img = generateImage(16, 16, [](int x, int y) -> Pixel
                                 {
        Pixel p;
        unsigned char val = static_cast<unsigned char>(y * 16 + x);
        p.r = val;
        p.g = val;
        p.b = val;
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_skewed_frequencies()
{
    TEST("Highly skewed frequencies (99% one value, 1% another)");
    BMPImage img = generateImage(10, 10, [](int x, int y) -> Pixel
                                 {
        Pixel p;
        // 99 pixels of value 100, 1 pixel of value 200
        if (x == 0 && y == 0) {
            p.r = 200; p.g = 200; p.b = 200;
        } else {
            p.r = 100; p.g = 100; p.b = 100;
        }
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_gradient_horizontal()
{
    TEST("Horizontal gradient 256x1");
    BMPImage img = generateImage(256, 1, [](int x, int) -> Pixel
                                 {
        Pixel p;
        p.r = static_cast<unsigned char>(x);
        p.g = static_cast<unsigned char>(255 - x);
        p.b = 128;
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_gradient_vertical()
{
    TEST("Vertical gradient 1x256");
    BMPImage img = generateImage(1, 256, [](int, int y) -> Pixel
                                 {
        Pixel p;
        p.r = static_cast<unsigned char>(y);
        p.g = 0;
        p.b = static_cast<unsigned char>(255 - y);
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_larger_image()
{
    TEST("Larger 100x100 RGB pattern");
    BMPImage img = generateImage(100, 100, [](int x, int y) -> Pixel
                                 {
        Pixel p;
        p.r = static_cast<unsigned char>((x * 3 + y * 7) % 256);
        p.g = static_cast<unsigned char>((x * 5 + y * 11) % 256);
        p.b = static_cast<unsigned char>((x * 13 + y * 17) % 256);
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_odd_dimensions()
{
    TEST("Odd dimensions 13x7");
    BMPImage img = generateImage(13, 7, [](int x, int y) -> Pixel
                                 {
        Pixel p;
        p.r = static_cast<unsigned char>((x * y) % 256);
        p.g = static_cast<unsigned char>((x + y * 2) % 256);
        p.b = static_cast<unsigned char>((x * 3 + y) % 256);
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_max_pixel_values()
{
    TEST("All pixels at max (255,255,255)");
    BMPImage img = generateImage(4, 4, [](int, int) -> Pixel
                                 {
        Pixel p;
        p.r = 255; p.g = 255; p.b = 255;
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_min_pixel_values()
{
    TEST("All pixels at min (0,0,0)");
    BMPImage img = generateImage(4, 4, [](int, int) -> Pixel
                                 {
        Pixel p;
        p.r = 0; p.g = 0; p.b = 0;
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_wide_image()
{
    TEST("Wide image 200x2");
    BMPImage img = generateImage(200, 2, [](int x, int y) -> Pixel
                                 {
        Pixel p;
        p.r = static_cast<unsigned char>((x + y * 100) % 256);
        p.g = static_cast<unsigned char>((x * 2 + y * 50) % 256);
        p.b = static_cast<unsigned char>((x * 3 + y * 25) % 256);
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

void test_tall_image()
{
    TEST("Tall image 2x200");
    BMPImage img = generateImage(2, 200, [](int x, int y) -> Pixel
                                 {
        Pixel p;
        p.r = static_cast<unsigned char>((x + y) % 256);
        p.g = static_cast<unsigned char>((x * 100 + y) % 256);
        p.b = static_cast<unsigned char>((y * 3) % 256);
        return p; });

    string err;
    if (roundTripTest(img, err))
        PASS();
    else
        FAIL(err);
}

// ============================================================================
// Invalid-input tests
// ============================================================================

// Helper: write raw bytes to a file
static void writeRawFile(const char *path, const void *data, size_t size)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        throw runtime_error("Failed to create test file");
    ssize_t n = write(fd, data, size);
    close(fd);
    if (n != static_cast<ssize_t>(size))
        throw runtime_error("Failed to write test file");
}

void test_invalid_compressed_magic()
{
    TEST("Invalid compressed file: bad magic");
    TempFile bad(".huf");
    TempFile out(".bmp");

    // Write a file with wrong magic
    char data[sizeof(FileHeader)] = {};
    data[0] = 'B';
    data[1] = 'A';
    data[2] = 'D';
    data[3] = '!';
    writeRawFile(bad.path(), data, sizeof(data));

    try
    {
        decompressImage(bad.path(), out.path());
        FAIL("Should have thrown for bad magic");
    }
    catch (const exception &e)
    {
        string msg = e.what();
        if (msg.find("magic") != string::npos || msg.find("format") != string::npos ||
            msg.find("Invalid") != string::npos)
            PASS();
        else
            FAIL(string("Wrong error message: ") + msg);
    }
}

void test_truncated_compressed_header()
{
    TEST("Invalid compressed file: truncated header");
    TempFile bad(".huf");
    TempFile out(".bmp");

    // Write only 10 bytes (header is 24 bytes)
    char data[10] = {'H', 'U', 'F', '1', 0, 0, 0, 0, 0, 0};
    writeRawFile(bad.path(), data, sizeof(data));

    try
    {
        decompressImage(bad.path(), out.path());
        FAIL("Should have thrown for truncated header");
    }
    catch (const exception &)
    {
        PASS();
    }
}

void test_truncated_frequency_table()
{
    TEST("Invalid compressed file: truncated frequency table");
    TempFile bad(".huf");
    TempFile out(".bmp");

    // Write a valid header claiming 10 unique symbols, but no frequency data
    FileHeader header{};
    header.magic[0] = 'H';
    header.magic[1] = 'U';
    header.magic[2] = 'F';
    header.magic[3] = '1';
    header.width = 4;
    header.height = 4;
    header.originalSize = 48;
    header.validBits = 0;
    header.uniqueSymbols = 10;
    writeRawFile(bad.path(), &header, sizeof(header));

    try
    {
        decompressImage(bad.path(), out.path());
        FAIL("Should have thrown for truncated freq table");
    }
    catch (const exception &)
    {
        PASS();
    }
}

void test_truncated_encoded_data()
{
    TEST("Invalid compressed file: truncated encoded data");
    TempFile huf(".huf");
    TempFile out(".bmp");

    // Create a valid header + freq table but no encoded data
    FileHeader header{};
    header.magic[0] = 'H';
    header.magic[1] = 'U';
    header.magic[2] = 'F';
    header.magic[3] = '1';
    header.width = 4;
    header.height = 4;
    header.originalSize = 48;
    header.validBits = 4;
    header.uniqueSymbols = 2;

    int fd = open(huf.path(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        throw runtime_error("Failed to create test file");

    write(fd, &header, sizeof(header));

    // Write 2 frequency entries
    FrequencyEntry e1{};
    e1.symbol = 0;
    e1.frequency = 24;
    write(fd, &e1, sizeof(e1));

    FrequencyEntry e2{};
    e2.symbol = 255;
    e2.frequency = 24;
    write(fd, &e2, sizeof(e2));

    // No encoded data!
    close(fd);

    try
    {
        decompressImage(huf.path(), out.path());
        FAIL("Should have thrown for truncated encoded data");
    }
    catch (const exception &)
    {
        PASS();
    }
}

void test_invalid_bmp_magic()
{
    TEST("Invalid BMP: bad signature");
    TempFile bad(".bmp");

    char data[64] = {};
    data[0] = 'X';
    data[1] = 'Y';
    writeRawFile(bad.path(), data, sizeof(data));

    try
    {
        readImage(bad.path());
        FAIL("Should have thrown for bad BMP signature");
    }
    catch (const exception &)
    {
        PASS();
    }
}

void test_truncated_bmp()
{
    TEST("Invalid BMP: truncated file (too short for headers)");
    TempFile bad(".bmp");

    // Just the BM signature, nothing else
    char data[4] = {'B', 'M', 0, 0};
    writeRawFile(bad.path(), data, sizeof(data));

    try
    {
        readImage(bad.path());
        FAIL("Should have thrown for truncated BMP");
    }
    catch (const exception &)
    {
        PASS();
    }
}

void test_nonexistent_file_compress()
{
    TEST("Nonexistent file: compress");
    TempFile out(".huf");

    try
    {
        compressImage("/tmp/nonexistent_test_file_12345.bmp", out.path());
        FAIL("Should have thrown for nonexistent file");
    }
    catch (const exception &)
    {
        PASS();
    }
}

void test_nonexistent_file_decompress()
{
    TEST("Nonexistent file: decompress");
    TempFile out(".bmp");

    try
    {
        decompressImage("/tmp/nonexistent_test_file_12345.huf", out.path());
        FAIL("Should have thrown for nonexistent file");
    }
    catch (const exception &)
    {
        PASS();
    }
}

void test_invalid_dimensions_in_header()
{
    TEST("Invalid compressed file: zero width");
    TempFile bad(".huf");
    TempFile out(".bmp");

    FileHeader header{};
    header.magic[0] = 'H';
    header.magic[1] = 'U';
    header.magic[2] = 'F';
    header.magic[3] = '1';
    header.width = 0;
    header.height = 4;
    header.originalSize = 0;
    header.validBits = 0;
    header.uniqueSymbols = 0;
    writeRawFile(bad.path(), &header, sizeof(header));

    try
    {
        decompressImage(bad.path(), out.path());
        FAIL("Should have thrown for zero width");
    }
    catch (const exception &)
    {
        PASS();
    }
}

void test_negative_dimensions_in_header()
{
    TEST("Invalid compressed file: negative height");
    TempFile bad(".huf");
    TempFile out(".bmp");

    FileHeader header{};
    header.magic[0] = 'H';
    header.magic[1] = 'U';
    header.magic[2] = 'F';
    header.magic[3] = '1';
    header.width = 4;
    header.height = -4;
    header.originalSize = 48;
    header.validBits = 0;
    header.uniqueSymbols = 1;
    writeRawFile(bad.path(), &header, sizeof(header));

    try
    {
        decompressImage(bad.path(), out.path());
        FAIL("Should have thrown for negative height");
    }
    catch (const exception &)
    {
        PASS();
    }
}

// ============================================================================
// Top-down BMP test
// ============================================================================

void test_topdown_bmp()
{
    TEST("Top-down BMP round-trip");

    // Manually create a top-down BMP (negative height in DIB header)
    int width = 4, height = 3;
    int rowSize = ((width * 3 + 3) / 4) * 4;
    uint32_t imageDataSize = static_cast<uint32_t>(rowSize * height);

    TempFile bmpFile(".bmp");

    // Write BMP with negative height
    int fd = open(bmpFile.path(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        throw runtime_error("Failed to create top-down BMP");

    // File header
    BMPFileHeader fh{};
    fh.signature = 0x4D42;
    fh.pixelDataOffset = 14 + 40;
    fh.fileSize = fh.pixelDataOffset + imageDataSize;
    write(fd, &fh, sizeof(fh));

    // DIB header with negative height (top-down)
    DIBHeader dh{};
    dh.headerSize = 40;
    dh.width = width;
    dh.height = -height; // top-down
    dh.planes = 1;
    dh.bitsPerPixel = 24;
    dh.compression = 0;
    dh.imageSize = imageDataSize;
    write(fd, &dh, sizeof(dh));

    // Pixel data: rows in top-to-bottom order, BGR format
    // Generate a known pattern
    vector<vector<Pixel>> rows(height, vector<Pixel>(width));
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            rows[y][x].r = static_cast<unsigned char>(y * 80 + x * 20);
            rows[y][x].g = static_cast<unsigned char>(y * 40 + x * 40);
            rows[y][x].b = static_cast<unsigned char>(y * 20 + x * 60);
        }
    }

    // Top-down: first row in file is row 0
    vector<unsigned char> rowBuf(rowSize, 0);
    for (int y = 0; y < height; y++)
    {
        memset(rowBuf.data(), 0, rowSize);
        for (int x = 0; x < width; x++)
        {
            rowBuf[x * 3] = rows[y][x].b;     // BGR
            rowBuf[x * 3 + 1] = rows[y][x].g;
            rowBuf[x * 3 + 2] = rows[y][x].r;
        }
        write(fd, rowBuf.data(), rowSize);
    }
    close(fd);

    // Read the top-down BMP
    BMPImage original = readImage(bmpFile.path());

    // Verify dimensions
    if (original.width != width || original.height != height)
    {
        FAIL("Top-down BMP dimensions wrong");
        return;
    }

    // Verify pixel data
    bool pixelsMatch = true;
    for (int y = 0; y < height && pixelsMatch; y++)
    {
        for (int x = 0; x < width && pixelsMatch; x++)
        {
            const Pixel &got = original.pixels[y * width + x];
            const Pixel &exp = rows[y][x];
            if (got.r != exp.r || got.g != exp.g || got.b != exp.b)
                pixelsMatch = false;
        }
    }

    if (!pixelsMatch)
    {
        FAIL("Top-down BMP pixel data mismatch after read");
        return;
    }

    // Now compress and decompress
    TempFile hufFile(".huf");
    TempFile outFile(".bmp");

    try
    {
        compressImage(bmpFile.path(), hufFile.path());
        decompressImage(hufFile.path(), outFile.path());
        BMPImage reconstructed = readImage(outFile.path());

        string err;
        if (compareImages(original, reconstructed, err))
            PASS();
        else
            FAIL(err);
    }
    catch (const exception &e)
    {
        FAIL(string("Exception: ") + e.what());
    }
}

void test_legacy_huf1_compatibility()
{
    TEST("Legacy HUF1 compatibility: decode archive without spatial prediction");

    // Generate a known image
    BMPImage original = generateImage(4, 3, [](int x, int y) -> Pixel {
        Pixel p;
        p.r = static_cast<unsigned char>(10 + x * 20 + y * 30);
        p.g = static_cast<unsigned char>(50 + x * 10 + y * 40);
        p.b = static_cast<unsigned char>(100 + x * 5 + y * 15);
        return p;
    });

    // Create raw pixel bytes (unpredicted)
    vector<unsigned char> data;
    for (const auto &p : original.pixels)
    {
        data.push_back(p.r);
        data.push_back(p.g);
        data.push_back(p.b);
    }

    // Calculate frequencies on raw unpredicted bytes
    vector<pair<int, int>> freq = calculateFrequency(data);
    adsc::HuffmanCodec::Frequencies huffmanFrequencies{};
    for (const auto &entry : freq)
        huffmanFrequencies[entry.first] = static_cast<uint64_t>(entry.second);
    const auto huffmanTree = adsc::HuffmanCodec::buildTree(huffmanFrequencies);
    const auto codes = adsc::HuffmanCodec::buildCodeTable(huffmanTree);

    TempFile huf1File(".huf");
    TempFile outFile(".bmp");

    int fd = open(huf1File.path(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        FAIL("Failed to create test HUF1 file");
        return;
    }

    // Manually serialize with legacy "HUF1" magic
    FileHeader header{};
    header.magic[0] = 'H';
    header.magic[1] = 'U';
    header.magic[2] = 'F';
    header.magic[3] = '1';
    header.width = original.width;
    header.height = original.height;
    header.originalSize = static_cast<int>(data.size());
    header.validBits = 0;
    header.uniqueSymbols = static_cast<int>(freq.size());

    writeFileHeader(fd, header);
    writeFrequencyTable(fd, freq);

    vector<int> encodedData(data.begin(), data.end());
    int validBits = BitWriter(fd, codes, encodedData);

    header.validBits = validBits;
    lseek(fd, offsetof(FileHeader, validBits), SEEK_SET);
    write(fd, &header.validBits, sizeof(header.validBits));
    close(fd);

    try
    {
        // Decompress should detect HUF1 and NOT apply inverse prediction
        decompressImage(huf1File.path(), outFile.path());
        BMPImage reconstructed = readImage(outFile.path());

        string err;
        if (compareImages(original, reconstructed, err))
            PASS();
        else
            FAIL(err);
    }
    catch (const exception &e)
    {
        FAIL(string("Exception: ") + e.what());
    }
}

// ============================================================================
// Main
// ============================================================================

int main()
{
    cout << "\n===============================================" << endl;
    cout << "  IMAGE COMPRESSION ROUND-TRIP TEST SUITE" << endl;
    cout << "===============================================" << endl;

    // Suppress compressor stdout during tests by redirecting cout
    // Actually, we want to see test output. The compressor prints to cout
    // which is fine for debugging. Let's keep it.

    // --- Valid round-trip tests ---
    cout << "\n--- Round-Trip Tests ---" << endl;

    test_small_rgb();
    test_small_rgb_8x6();
    test_row_padding_width3();
    test_row_padding_width5();
    test_row_padding_width7();
    test_1x1_pixel();
    test_single_symbol_uniform();
    test_single_symbol_1x1();
    test_two_symbols();
    test_two_symbols_rgb();
    test_all_256_grayscale_values();
    test_skewed_frequencies();
    test_gradient_horizontal();
    test_gradient_vertical();
    test_larger_image();
    test_odd_dimensions();
    test_max_pixel_values();
    test_min_pixel_values();
    test_wide_image();
    test_tall_image();
    test_topdown_bmp();
    test_legacy_huf1_compatibility();

    // --- Invalid-input tests ---
    cout << "\n--- Invalid Input Tests ---" << endl;

    test_invalid_compressed_magic();
    test_truncated_compressed_header();
    test_truncated_frequency_table();
    test_truncated_encoded_data();
    test_invalid_bmp_magic();
    test_truncated_bmp();
    test_nonexistent_file_compress();
    test_nonexistent_file_decompress();
    test_invalid_dimensions_in_header();
    test_negative_dimensions_in_header();

    // --- Summary ---
    cout << "\n===============================================" << endl;
    cout << "  RESULTS: " << g_passed << " passed, " << g_failed
         << " failed, " << g_total << " total" << endl;
    cout << "===============================================" << endl;

    return g_failed > 0 ? 1 : 0;
}
