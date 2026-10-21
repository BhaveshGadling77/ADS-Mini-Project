#ifndef BMP_READER_H
#define BMP_READER_H

#include <cstdint>
#include <vector>
#pragma pack(push, 1)
struct Pixel
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

struct BMPFileHeader
{
    uint16_t signature;
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t pixelDataOffset;
};

struct DIBHeader
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

struct BMPImage
{
    int width;
    int height;
    std::vector<Pixel> pixels;
};

int openBMP(const char *filename);
BMPFileHeader readBMPFileHeader(int fd);
DIBHeader readDIBHeader(int fd);
void validateBMP(const BMPFileHeader &fileHeader, const DIBHeader &dibHeader);
std::vector<Pixel> readPixelData(int fd, const BMPFileHeader &fileHeader, const DIBHeader &dibHeader);
BMPImage readImage(const char *filename);



#endif
