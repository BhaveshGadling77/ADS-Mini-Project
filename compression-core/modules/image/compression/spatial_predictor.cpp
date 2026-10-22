#include "spatial_predictor.h"

#include <stdexcept>
#include <cstddef>

void applySpatialPrediction(std::vector<unsigned char> &data, int width, int height)
{
    if (width < 0 || height < 0)
    {
        throw std::invalid_argument("Invalid dimensions for spatial prediction");
    }

    if (width == 0 || height == 0)
    {
        if (!data.empty())
        {
            throw std::invalid_argument("Non-empty data provided with zero dimensions");
        }
        return;
    }

    size_t expectedSize = static_cast<size_t>(width) * height * 3;
    if (data.size() != expectedSize)
    {
        throw std::invalid_argument("Data size does not match width * height * 3");
    }

    // Traverse each row from right to left (x = width - 1 down to 1).
    // Moving backwards ensures data[x - 1] still holds the original unpredicted pixel
    // value when computing data[x] = (current - left) modulo 256.
    // Pixel x = 0 is left untouched (residual = current).
    for (int y = 0; y < height; ++y)
    {
        size_t rowStart = static_cast<size_t>(y) * width * 3;

        for (int x = width - 1; x >= 1; --x)
        {
            size_t currIdx = rowStart + static_cast<size_t>(x) * 3;
            size_t leftIdx = rowStart + static_cast<size_t>(x - 1) * 3;

            // R, G, B channels predicted independently with unsigned byte wraparound (modulo 256)
            data[currIdx]     = static_cast<unsigned char>(data[currIdx]     - data[leftIdx]);
            data[currIdx + 1] = static_cast<unsigned char>(data[currIdx + 1] - data[leftIdx + 1]);
            data[currIdx + 2] = static_cast<unsigned char>(data[currIdx + 2] - data[leftIdx + 2]);
        }
    }
}

void applyInverseSpatialPrediction(std::vector<unsigned char> &data, int width, int height)
{
    if (width < 0 || height < 0)
    {
        throw std::invalid_argument("Invalid dimensions for spatial inverse prediction");
    }

    if (width == 0 || height == 0)
    {
        if (!data.empty())
        {
            throw std::invalid_argument("Non-empty data provided with zero dimensions");
        }
        return;
    }

    size_t expectedSize = static_cast<size_t>(width) * height * 3;
    if (data.size() != expectedSize)
    {
        throw std::invalid_argument("Data size does not match width * height * 3");
    }

    // Traverse each row from left to right (x = 1 up to width - 1).
    // Moving forwards ensures data[x - 1] has already been restored to its original value
    // when computing data[x] = (left + residual) modulo 256.
    // Pixel x = 0 is left untouched (original = residual).
    for (int y = 0; y < height; ++y)
    {
        size_t rowStart = static_cast<size_t>(y) * width * 3;

        for (int x = 1; x < width; ++x)
        {
            size_t currIdx = rowStart + static_cast<size_t>(x) * 3;
            size_t leftIdx = rowStart + static_cast<size_t>(x - 1) * 3;

            // R, G, B channels restored independently with unsigned byte wraparound (modulo 256)
            data[currIdx]     = static_cast<unsigned char>(data[leftIdx]     + data[currIdx]);
            data[currIdx + 1] = static_cast<unsigned char>(data[leftIdx + 1] + data[currIdx + 1]);
            data[currIdx + 2] = static_cast<unsigned char>(data[leftIdx + 2] + data[currIdx + 2]);
        }
    }
}
