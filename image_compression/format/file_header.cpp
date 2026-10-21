#include "file_header.h"

#include <unistd.h>
#include <stdexcept>
#include <cstring>

using namespace std;

// Helper: read exactly `size` bytes, retrying on EINTR.
static void readAll(int fd, void *buf, size_t size)
{
    unsigned char *ptr = static_cast<unsigned char *>(buf);
    size_t total = 0;

    while (total < size)
    {
        ssize_t n = read(fd, ptr + total, size - total);

        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            throw runtime_error("Failed to read from compressed file");
        }

        if (n == 0)
            throw runtime_error("Unexpected EOF reading compressed file");

        total += static_cast<size_t>(n);
    }
}

// Helper: write exactly `size` bytes, retrying on EINTR.
static void writeAllBytes(int fd, const void *buf, size_t size)
{
    const unsigned char *ptr = static_cast<const unsigned char *>(buf);
    size_t total = 0;

    while (total < size)
    {
        ssize_t n = write(fd, ptr + total, size - total);

        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            throw runtime_error("Failed to write to compressed file");
        }

        if (n == 0)
            throw runtime_error("write returned 0");

        total += static_cast<size_t>(n);
    }
}

FileHeader readFileHeader(int fd)
{
    FileHeader header{};

    readAll(fd, &header, sizeof(header));

    if (memcmp(header.magic, "HUF1", 4) != 0)
    {
        throw runtime_error("Invalid compressed file format (bad magic)");
    }

    if (header.width <= 0 || header.height <= 0)
    {
        throw runtime_error("Invalid image dimensions in compressed file header");
    }

    if (header.originalSize < 0)
    {
        throw runtime_error("Invalid original size in compressed file header");
    }

    if (header.uniqueSymbols < 0 || header.uniqueSymbols > 256)
    {
        throw runtime_error("Invalid unique symbol count in compressed file header");
    }

    if (header.validBits < 0 || header.validBits > 8)
    {
        throw runtime_error("Invalid validBits in compressed file header");
    }

    return header;
}

vector<pair<int, int>> readFrequencyTable(int fd, int uniqueSymbols)
{
    vector<pair<int, int>> freq;

    for (int i = 0; i < uniqueSymbols; i++)
    {
        FrequencyEntry entry{};

        readAll(fd, &entry, sizeof(entry));

        if (entry.frequency <= 0)
        {
            throw runtime_error("Invalid frequency in frequency table");
        }

        freq.push_back({entry.symbol, entry.frequency});
    }

    return freq;
}

void writeFileHeader(int fd, const FileHeader &header)
{
    writeAllBytes(fd, &header, sizeof(header));
}

void writeFrequencyTable(int fd, const vector<pair<int, int>> &freq)
{
    for (const auto &p : freq)
    {
        FrequencyEntry entry{};

        entry.symbol = static_cast<unsigned char>(p.first);
        entry.frequency = p.second;

        writeAllBytes(fd, &entry, sizeof(entry));
    }
}