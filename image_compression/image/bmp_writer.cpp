#include "bmp_writer.h"

#include <fcntl.h>
#include <unistd.h>
#include <cstdint>
#include <vector>
#include <stdexcept>
#include <cerrno>
#include <cstring>

using namespace std;

#pragma pack(push, 1)

struct BMPOutputFileHeader
{
    uint16_t signature;
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t pixelDataOffset;
};

struct BMPOutputDIBHeader
{
    uint32_t headerSize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPixelsPerMeter;
    int32_t yPixelsPerMeter;
    uint32_t colorsUsed;
    uint32_t importantColors;
};

#pragma pack(pop)

static_assert(sizeof(BMPOutputFileHeader) == 14);
static_assert(sizeof(BMPOutputDIBHeader) == 40);

static void writeAll(int fd, const void *buffer, size_t size)
{
    const unsigned char *ptr =
        static_cast<const unsigned char *>(buffer);

    size_t written = 0;

    while (written < size)
    {
        ssize_t n = write(fd, ptr + written, size - written);

        if (n <= 0)
            throw runtime_error("Failed to write BMP file");

        written += static_cast<size_t>(n);
    }
}

void writeBMP(const char *filename, const BMPImage &image)
{
    if (image.width <= 0 || image.height <= 0 ||
        image.pixels.size() !=
            static_cast<size_t>(image.width) * image.height)
    {
        throw runtime_error("Invalid BMP image dimensions or pixels");
    }

    int rowSize = ((image.width * 3 + 3) / 4) * 4;
    uint32_t imageSize =
        static_cast<uint32_t>(rowSize) * image.height;

    BMPOutputFileHeader fileHeader{};
    fileHeader.signature = 0x4D42;
    fileHeader.pixelDataOffset =
        sizeof(BMPOutputFileHeader) + sizeof(BMPOutputDIBHeader);
    fileHeader.fileSize = fileHeader.pixelDataOffset + imageSize;

    BMPOutputDIBHeader dibHeader{};
    dibHeader.headerSize = sizeof(BMPOutputDIBHeader);
    dibHeader.width = image.width;
    dibHeader.height = image.height;
    dibHeader.planes = 1;
    dibHeader.bitsPerPixel = 24;
    dibHeader.compression = 0;
    dibHeader.imageSize = imageSize;

    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1) {
    throw runtime_error(
        string("Failed to open output BMP file: ") + filename +
        " | Reason: " + strerror(errno)
        );
    }
    try
    {
        writeAll(fd, &fileHeader, sizeof(fileHeader));
        writeAll(fd, &dibHeader, sizeof(dibHeader));

        vector<unsigned char> row(rowSize, 0);

        for (int y = image.height - 1; y >= 0; y--)
        {
            for (int x = 0; x < image.width; x++)
            {
                const Pixel &pixel =
                    image.pixels[y * image.width + x];

                row[x * 3] = pixel.b;
                row[x * 3 + 1] = pixel.g;
                row[x * 3 + 2] = pixel.r;
            }

            writeAll(fd, row.data(), row.size());
        }

        if (close(fd) == -1)
            throw runtime_error("Failed to close output BMP file");
    }
    catch (...)
    {
        close(fd);
        throw;
    }
}