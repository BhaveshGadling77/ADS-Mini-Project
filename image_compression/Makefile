CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
TARGET = compressor

SOURCES = \
	main.cpp \
	image/bmp_reader.cpp \
	compression/data_preprocessor.cpp \
	compression/frequency.cpp \
	compression/huffman.cpp \
	compression/spatial_predictor.cpp \
	format/file_header.cpp \
	compression/bit_writer.cpp \
	compression/encoder.cpp \
	compression/decoder.cpp \
	compression/bit_reader.cpp \
	image/bmp_writer.cpp \

OBJECTS = $(SOURCES:.cpp=.o)

# Shared library sources (everything except main.cpp)
LIB_SOURCES = \
	image/bmp_reader.cpp \
	compression/data_preprocessor.cpp \
	compression/frequency.cpp \
	compression/huffman.cpp \
	compression/spatial_predictor.cpp \
	format/file_header.cpp \
	compression/bit_writer.cpp \
	compression/encoder.cpp \
	compression/decoder.cpp \
	compression/bit_reader.cpp \
	image/bmp_writer.cpp

LIB_OBJECTS = $(LIB_SOURCES:.cpp=.o)

.PHONY: all clean run test test_predictor

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) compare_images test_roundtrip tests/test_roundtrip.o test_spatial_predictor tests/test_spatial_predictor.o

run: $(TARGET)
	./$(TARGET)

compare_images: compare_images.cpp image/bmp_reader.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

test_roundtrip: tests/test_roundtrip.o $(LIB_OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

test_spatial_predictor: tests/test_spatial_predictor.o compression/spatial_predictor.o
	$(CXX) $(CXXFLAGS) $^ -o $@

test_predictor: test_spatial_predictor
	./test_spatial_predictor

test: test_roundtrip test_spatial_predictor
	./test_roundtrip
	./test_spatial_predictor