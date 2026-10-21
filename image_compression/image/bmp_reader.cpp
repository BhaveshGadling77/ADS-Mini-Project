#include "bmp_reader.h"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <cmath>
using namespace std;

static_assert(sizeof(BMPFileHeader) == 14);
static_assert(sizeof(DIBHeader) == 40);

int openBMP(const char *filename)
{
    int fd = open(filename, O_RDONLY);

    if (fd < 0)
    {
        throw runtime_error(string("Failed to open BMP file: ") + filename);
    }

    return fd;
}

BMPFileHeader readBMPFileHeader(int fd)
{
    BMPFileHeader header;

    ssize_t bytesRead = read(fd, &header, sizeof(header));

    if (bytesRead != sizeof(header))
    {
        throw runtime_error("Failed to read BMP file header");
    }

    if (header.signature != 0x4D42)
    {
        throw runtime_error("Not a BMP file");
    }

    return header;
}

DIBHeader readDIBHeader(int fd)
{
    DIBHeader header;

    ssize_t bytesRead = read(fd, &header, sizeof(header));

    if (bytesRead != sizeof(header))
    {
        throw runtime_error("Failed to read DIB header");
    }

    if (header.headerSize < 40)
    {
        throw runtime_error("Unsupported DIB header");
    }

    return header;
}

void validateBMP(const BMPFileHeader &fileHeader, const DIBHeader &dibHeader)
{
    if (fileHeader.signature != 0x4D42)
    {
        throw runtime_error("Not a BMP file");
    }

    if (dibHeader.headerSize < 40)
    {
        throw runtime_error("Unsupported DIB header");
    }

    if (dibHeader.planes != 1)
    {
        throw runtime_error("Unsupported number of planes");
    }

    if (dibHeader.bitsPerPixel != 24)
    {
        throw runtime_error("Only 24-bit BMP files are supported");
    }

    if (dibHeader.compression != 0)
    {
        throw runtime_error("Compressed BMP files are not supported");
    }

    // Validate dimensions
    int32_t absHeight = dibHeader.height < 0 ? -dibHeader.height : dibHeader.height;
    if (dibHeader.width <= 0 || absHeight <= 0)
    {
        throw runtime_error("Invalid BMP dimensions");
    }
}

vector<Pixel> readPixelData(int fd, const BMPFileHeader &fileHeader, const DIBHeader &dibHeader)
{
    int width = dibHeader.width;
    // Handle top-down BMPs (negative height)
    bool topDown = dibHeader.height < 0;
    int height = topDown ? -dibHeader.height : dibHeader.height;

    int rowSize = ((width * 3 + 3) / 4) * 4;

    vector<Pixel> pixels(static_cast<size_t>(width) * height);
    vector<unsigned char> row(rowSize);

    if (lseek(fd, fileHeader.pixelDataOffset, SEEK_SET) == -1)
    {
        throw runtime_error("Failed to seek to pixel data");
    }

    for (int rowIndex = 0; rowIndex < height; rowIndex++)
    {
        ssize_t bytesRead = read(fd, row.data(), rowSize);

        if (bytesRead != rowSize)
        {
            throw runtime_error("Failed to read pixel data");
        }

        // Bottom-up BMP: first row in file is last row in image
        // Top-down BMP: first row in file is first row in image
        int destinationRow = topDown ? rowIndex : (height - 1 - rowIndex);

        for (int col = 0; col < width; col++)
        {
            int index = col * 3;

            // BMP stores BGR; convert to RGB
            pixels[destinationRow * width + col].b = row[index];
            pixels[destinationRow * width + col].g = row[index + 1];
            pixels[destinationRow * width + col].r = row[index + 2];
        }
    }

    return pixels;
}

BMPImage readImage(const char *filename)
{
    int fd = openBMP(filename);

    try
    {
        BMPFileHeader fileHeader = readBMPFileHeader(fd);
        DIBHeader dibHeader = readDIBHeader(fd);

        validateBMP(fileHeader, dibHeader);

        vector<Pixel> pixels = readPixelData(fd, fileHeader, dibHeader);

        close(fd);

        BMPImage image;
        image.width = dibHeader.width;
        // Store absolute height in BMPImage
        image.height = dibHeader.height < 0 ? -dibHeader.height : dibHeader.height;
        image.pixels = pixels;

        return image;
    }
    catch (...)
    {
        close(fd);
        throw;
    }
}