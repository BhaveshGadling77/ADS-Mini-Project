CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
TARGET = compressor

SOURCES = \
	main.cpp \
	image/bmp_reader.cpp \
	compression/data_preprocessor.cpp \
	compression/frequency.cpp \
	compression/huffman.cpp \
	format/file_header.cpp \
	compression/bit_writer.cpp \
	compression/encoder.cpp \
	compression/decoder.cpp \
	compression/bit_reader.cpp \
	image/bmp_writer.cpp \

OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) compare_images

run: $(TARGET)
	./$(TARGET)

compare_images: compare_images.cpp image/bmp_reader.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@