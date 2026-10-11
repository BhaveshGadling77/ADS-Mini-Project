#include "bit_writer.h"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

void BitWriter(int fd, const vector<string> &codes, const vector<int> &data)
{
    Buffer buffer;

    buffer.byte = 0;
    buffer.bitCount = 0;
    buffer.size = 8;

    for (int i = 0; i < data.size(); i++)
    {
        for (char ch : codes[data[i]])
        {
            if (buffer.bitCount == buffer.size)
            {
                write(fd, &buffer.byte, 1);

                buffer.byte = 0;
                buffer.bitCount = 0;
            }

            buffer.byte = (buffer.byte << 1) | (ch - '0');
            buffer.bitCount++;
        }
    }

    if (buffer.bitCount > 0)
    {
        while (buffer.bitCount < buffer.size)
        {
            buffer.byte = buffer.byte << 1;
            buffer.bitCount++;
        }

        write(fd, &buffer.byte, 1);
    }

    close(fd);
}
