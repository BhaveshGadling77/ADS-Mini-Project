
#include <iostream>
#include <exception>
#include "image/bmp_reader.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: ./compare_images <original.bmp> <reconstructed.bmp>\n";
        return 1;
    }

    try
    {
        BMPImage original = readImage(argv[1]);
        BMPImage reconstructed = readImage(argv[2]);

        if (original.width != reconstructed.width ||
            original.height != reconstructed.height)
        {
            cerr << "FAIL: Image dimensions differ.\n";
            cerr << "Original: " << original.width << "x"
                 << original.height << '\n';
            cerr << "Reconstructed: " << reconstructed.width << "x"
                 << reconstructed.height << '\n';
            return 1;
        }

        if (original.pixels.size() != reconstructed.pixels.size())
        {
            cerr << "FAIL: Pixel counts differ.\n";
            return 1;
        }

        for (size_t i = 0; i < original.pixels.size(); i++)
        {
            const Pixel &a = original.pixels[i];
            const Pixel &b = reconstructed.pixels[i];

            if (a.r != b.r || a.g != b.g || a.b != b.b)
            {
                cerr << "FAIL: Pixel mismatch at index " << i << '\n';
                cerr << "Original RGB: "
                     << static_cast<int>(a.r) << ","
                     << static_cast<int>(a.g) << ","
                     << static_cast<int>(a.b) << '\n';
                cerr << "Reconstructed RGB: "
                     << static_cast<int>(b.r) << ","
                     << static_cast<int>(b.g) << ","
                     << static_cast<int>(b.b) << '\n';
                return 1;
            }
        }

        cout << "PASS: All " << original.pixels.size()
             << " pixels match exactly.\n";
        return 0;
    }
    catch (const exception &e)
    {
        cerr << "ERROR: " << e.what() << '\n';
        return 1;
    }
}
