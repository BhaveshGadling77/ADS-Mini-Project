#ifndef SPATIAL_PREDICTOR_H
#define SPATIAL_PREDICTOR_H

#include <vector>

// Forward left-neighbor predictor (applied independently to R, G, B channels):
//   For pixel x = 0 in each row: residual = current
//   For pixel x > 0: residual = (current - left) modulo 256
// Operates in-place on interleaved RGB byte stream (size must equal width * height * 3).
void applySpatialPrediction(std::vector<unsigned char> &data, int width, int height);

// Inverse left-neighbor predictor:
//   For pixel x = 0 in each row: current = residual
//   For pixel x > 0: current = (left + residual) modulo 256
// Operates in-place on interleaved residual byte stream.
void applyInverseSpatialPrediction(std::vector<unsigned char> &data, int width, int height);

#endif // SPATIAL_PREDICTOR_H
