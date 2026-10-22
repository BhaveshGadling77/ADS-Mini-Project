#include "data_preprocessor.h"

// TODO: Flatten pixels into a byte stream.
// This should convert BMPImage -> std::vector<unsigned char> without changing
// any existing pixel-order logic already written elsewhere.
// prepareData()
// │
// ├── extractRGB()
// │
// ├── flattenPixels()
// │
// └── applyTransformation()

std::vector<unsigned char> prepareData(const BMPImage &image)
{
    std::vector<unsigned char> data;
    data.reserve(image.pixels.size() * 3);

    for (const Pixel &pixel : image.pixels)
    {
        data.push_back(pixel.r);
        data.push_back(pixel.g);
        data.push_back(pixel.b);
    }

    return data;
}
