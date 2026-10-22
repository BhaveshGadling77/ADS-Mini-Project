#ifndef BIT_READER_H
#define BIT_READER_H

#include <string>
#include <vector>

struct ReadBuffer
{
    unsigned char byte;
    int bitCount;
    int size;
};

int readBit(int fd, ReadBuffer &buffer);

#endif
