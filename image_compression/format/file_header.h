#ifndef FILE_HEADER_H
#define FILE_HEADER_H

#include <vector>
#include <utility>

struct FileHeader
{
    char magic[4];

    int width;
    int height;

    int originalSize;

    int validBits;

    int uniqueSymbols;
};

struct FrequencyEntry
{
    unsigned char symbol;
    int frequency;
};

FileHeader readFileHeader(int fd);
std::vector<std::pair<int, int>> readFrequencyTable(int fd, int uniqueSymbols);

void writeFileHeader(int fd, const FileHeader &header);
void writeFrequencyTable(int fd, const std::vector<std::pair<int, int>> &freq);

#endif