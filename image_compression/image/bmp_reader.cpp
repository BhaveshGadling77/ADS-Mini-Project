#include "bmp_reader.h"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstdint>
#include <stdexcept>
using namespace std;

// TODO: Move and finalize the BMP parsing logic here.
// This module is responsible only for reading the BMP file and returning
// image information and pixel data.

static_assert(sizeof(BMPFileHeader) == 14);
static_assert(sizeof(DIBHeader) == 40);

int openBMP(const char *filename)
{
    int fd = open(filename, O_RDONLY);

    if (fd < 0)
    {
        cout << "Error opening BMP file" << endl;
        exit(EXIT_FAILURE);
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
    cout << hex << "Signature read: 0x"
     << header.signature << dec << endl;
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
}

vector<Pixel> readPixelData(int fd, const BMPFileHeader &fileHeader, const DIBHeader &dibHeader)
{
    int width = dibHeader.width;
    int height = dibHeader.height;

    int rowSize = ((width * 3 + 3) / 4) * 4;

    vector<Pixel> pixels(width * height);
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

        int destinationRow = height - 1 - rowIndex;

        for (int col = 0; col < width; col++)
        {
            int index = col * 3;

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

    BMPFileHeader fileHeader = readBMPFileHeader(fd);
    DIBHeader dibHeader = readDIBHeader(fd);

    validateBMP(fileHeader, dibHeader);

    vector<Pixel> pixels = readPixelData(fd, fileHeader, dibHeader);

    close(fd);

    BMPImage image;
    image.width = dibHeader.width;
    image.height = dibHeader.height;
    image.pixels = pixels;

    return image;
}