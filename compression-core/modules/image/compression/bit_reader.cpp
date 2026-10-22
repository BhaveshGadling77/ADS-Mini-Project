#include "bit_reader.h"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

using namespace std;

int readBit(int fd, ReadBuffer &readBuffer)
{
    if (readBuffer.bitCount == 0)
    {
        ssize_t bytesRead = read(fd, &readBuffer.byte, 1);

        if (bytesRead != 1)
            return -1;

        readBuffer.bitCount = readBuffer.size;
    }

    int bit = (readBuffer.byte >> (readBuffer.bitCount - 1)) & 1;

    readBuffer.bitCount--;

    return bit;
}
