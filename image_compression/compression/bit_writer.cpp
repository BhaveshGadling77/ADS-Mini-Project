#include "bit_writer.h"
#include <unistd.h>
#include <cerrno>
#include <stdexcept>
#include <vector>
#include <string>

using namespace std;

// Write exactly `size` bytes, retrying on EINTR and partial writes.
static void writeAll(int fd, const void *buf, size_t size)
{
    const unsigned char *ptr = static_cast<const unsigned char *>(buf);
    size_t written = 0;

    while (written < size)
    {
        ssize_t n = write(fd, ptr + written, size - written);

        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            throw runtime_error("BitWriter: write failed");
        }

        if (n == 0)
            throw runtime_error("BitWriter: write returned 0");

        written += static_cast<size_t>(n);
    }
}

int BitWriter(int fd, const vector<string> &codes, const vector<int> &data)
{
    Buffer buffer;
    buffer.byte = 0;
    buffer.bitCount = 0;

    for (size_t i = 0; i < data.size(); i++)
    {
        const string &code = codes[data[i]];
        for (char ch : code)
        {
            // Pack bits MSB-first: shift left and OR in the new bit
            buffer.byte = static_cast<unsigned char>((buffer.byte << 1) | (ch - '0'));
            buffer.bitCount++;

            if (buffer.bitCount == 8)
            {
                writeAll(fd, &buffer.byte, 1);
                buffer.byte = 0;
                buffer.bitCount = 0;
            }
        }
    }

    // Write the final partially filled byte, padded with zeros in the LSB positions
    if (buffer.bitCount > 0)
    {
        int validBits = buffer.bitCount;
        buffer.byte = static_cast<unsigned char>(buffer.byte << (8 - buffer.bitCount));
        writeAll(fd, &buffer.byte, 1);
        return validBits;
    }

    return 0; // No partial byte — all bytes were full (or no bits written)
}
