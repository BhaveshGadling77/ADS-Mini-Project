#include "encoder.h"

#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <utility>

#include "../image/bmp_reader.h"
#include "data_preprocessor.h"
#include "frequency.h"
#include "huffman.h"
#include "../format/file_header.h"
#include "bit_writer.h"

using namespace std;

void compressImage(
    const char *inputFilename,
    const char *outputFilename)
{
    // 1. Read image
    BMPImage image = readImage(inputFilename);

    cout << "Width: " << image.width << "\n";
    cout << "Height: " << image.height << "\n";
    cout << "Pixels: " << image.pixels.size() << "\n";

    for (int i = 0; i < 5 && i < image.pixels.size(); i++)
    {
        cout << "Pixel " << i << ": "
             << (int)image.pixels[i].r << " "
             << (int)image.pixels[i].g << " "
             << (int)image.pixels[i].b << "\n";
    }

    // 2. Prepare byte data
    vector<unsigned char> data = prepareData(image);
    cout << "Data bytes: " << data.size() << "\n";

    for (int i = 0; i < 15 && i < data.size(); i++)
    {
        cout << (int)data[i] << " ";
    }

    cout << "\n";

    // 3. Calculate frequencies
    vector<pair<int, int>> freq = calculateFrequency(data);

    cout << "Unique bytes: " << freq.size() << "\n";

    for (auto entry : freq)
    {
        cout << entry.first << " -> " << entry.second << "\n";
    }

    // 4. Generate Huffman codes
    vector<string> codes(256, "");
    huffmanEncoding(freq, codes);

    cout << "\nHuffman Codes:\n";

    for (auto p : freq)
    {
        cout << p.first << " -> " << codes[p.first] << "\n";
    }

    // 5. Prepare compressed file header
    FileHeader header;

    header.magic[0] = 'H';
    header.magic[1] = 'U';
    header.magic[2] = 'F';
    header.magic[3] = '1';

    header.width = image.width;
    header.height = image.height;
    header.originalSize = data.size();
    header.validBits = 0;
    header.uniqueSymbols = freq.size();

    // 6. Open compressed file
    int fd = open(
        outputFilename,
        O_WRONLY | O_CREAT | O_TRUNC,
        0644);

    if (fd == -1)
    {
        throw runtime_error("Failed to open output file");
    }

    // 7. Write header
    writeFileHeader(fd, header);

    // 8. Write frequency table
    writeFrequencyTable(fd, freq);

    // 9. Write Huffman encoded data
    vector<int> encodedData(data.begin(), data.end());
    BitWriter(fd, codes, encodedData);

    cout << "Valid bits: " << header.validBits << "\n";

    // 10. Update validBits in the header
    lseek(fd, 16, SEEK_SET);
    write(fd, &header.validBits, sizeof(header.validBits));

    // 11. Close compressed file
    close(fd);
}
