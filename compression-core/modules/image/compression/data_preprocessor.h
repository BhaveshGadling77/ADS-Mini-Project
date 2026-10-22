#ifndef DATA_PREPROCESSOR_H
#define DATA_PREPROCESSOR_H

#include <vector>
#include "../image/bmp_reader.h"

std::vector<unsigned char> prepareData(const BMPImage &image);

#endif
