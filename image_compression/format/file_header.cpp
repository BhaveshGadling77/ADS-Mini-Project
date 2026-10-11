#include "file_header.h"

#include <unistd.h>
#include <stdexcept>
#include <cstring>

using namespace std;

FileHeader readFileHeader(int fd)
{
    FileHeader header{};

    ssize_t bytesRead = read(fd, &header, sizeof(header));

    if (bytesRead != sizeof(header))
    {
        throw runtime_error("Failed to read file header");
    }

    if (memcmp(header.magic, "HUF1", 4) != 0)
    {
        throw runtime_error("Invalid compressed file format");
    }

    return header;
}

vector<pair<int, int>> readFrequencyTable(int fd, int uniqueSymbols)
{
    vector<pair<int, int>> freq;

    for (int i = 0; i < uniqueSymbols; i++)
    {
        FrequencyEntry entry{};

        ssize_t bytesRead = read(fd, &entry, sizeof(entry));

        if (bytesRead != sizeof(entry))
        {
            throw runtime_error("Failed to read frequency table");
        }

        freq.push_back({entry.symbol, entry.frequency});
    }

    return freq;
}

void writeFileHeader(int fd, const FileHeader &header)
{
    ssize_t bytesWritten = write(fd, &header, sizeof(header));

    if (bytesWritten != sizeof(header))
    {
        throw runtime_error("Failed to write file header");
    }
}

void writeFrequencyTable(int fd, const vector<pair<int, int>> &freq)
{
    for (auto p : freq)
    {
        FrequencyEntry entry{};

        entry.symbol = p.first;
        entry.frequency = p.second;

        ssize_t bytesWritten = write(fd, &entry, sizeof(entry));

        if (bytesWritten != sizeof(entry))
        {
            throw runtime_error("Failed to write frequency table");
        }
    }
}