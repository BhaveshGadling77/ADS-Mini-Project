#ifndef BIT_WRITER_H
#define BIT_WRITER_H

#include <string>
#include <vector>

struct Buffer {
    char byte;
    int bitCount;
    int size;
};

void BitWriter(int fd, const std::vector<std::string> &codes, const std::vector<int> &data);

#endif
