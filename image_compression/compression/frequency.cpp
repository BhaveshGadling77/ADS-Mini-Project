#include "frequency.h"

// This logic is intentionally kept minimal and matches the current
// frequency-table behavior already used by the project.

std::vector<std::pair<int, int>> calculateFrequency(const std::vector<unsigned char> &data)
{
    int frequency[256] = {0};

    for (unsigned char value : data)
    {
        frequency[value]++;
    }

    std::vector<std::pair<int, int>> result;
    for (int i = 0; i < 256; ++i)
    {
        if (frequency[i] > 0)
        {
            result.push_back({i, frequency[i]});
        }
    }

    return result;
}
